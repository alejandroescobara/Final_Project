#include "Doppel.h"

#include <QtMath>
#include <algorithm>
#include <cmath>
#include <memory>

#include "ArcAttack.h"
#include "GameplayConfig.h"
#include "PhysicsMotor.h"
#include "PlanarPhysicsMotor.h"
#include "Player.h"

namespace {
  constexpr double REWARD_HIT_PLAYER = 3.0;
  constexpr double REWARD_PARRY_SUCCESS = 4.0;
  constexpr double REWARD_MISSED_ATTACK = -0.5;
  constexpr double REWARD_INVALID_ACTION = -0.25;
  constexpr double REWARD_WAS_PARRIED = -2.5;
  constexpr double REWARD_TAKEN_DAMAGE = -2.0;
  constexpr double REWARD_DEATH = -6.0;

  double signedAngleDegrees(QVector2D from, QVector2D to) {
    if (from.isNull() || to.isNull()) return 0.0;

    const double dot = static_cast<double>(from.x()) * to.x() +
                       static_cast<double>(from.y()) * to.y();
    const double cross = static_cast<double>(from.x()) * to.y() -
                         static_cast<double>(from.y()) * to.x();
    return qRadiansToDegrees(std::atan2(cross, dot));
  }

  QVector2D rotateVectorDegrees(QVector2D baseDir, double angleDegrees) {
    if (baseDir.isNull()) {
      baseDir = QVector2D(1.0f, 0.0f);
    } else {
      baseDir.normalize();
    }

    const double rad = qDegreesToRadians(angleDegrees);
    const double c = std::cos(rad);
    const double s = std::sin(rad);

    return QVector2D(static_cast<float>(baseDir.x() * c - baseDir.y() * s),
                     static_cast<float>(baseDir.x() * s + baseDir.y() * c));
  }

  bool isMoveAction(doppelAction action) {
    return action == doppelAction::MOVE_FORWARD ||
           action == doppelAction::MOVE_AWAY ||
           action == doppelAction::MOVE_RIGHT ||
           action == doppelAction::MOVE_LEFT;
  }
}  // namespace

Doppel::Doppel(QVector2D initialPosition, int level)
    : Character(initialPosition,
                GameplayConfig::doppel.baseHp,
                GameplayConfig::doppel.baseSpeed,
                GameplayConfig::doppel.weight,
                std::make_unique<PlanarPhysicsMotor>(GameplayConfig::movementFriction),
                Faction::Enemy),
      roll(GameplayConfig::doppelRollSpeed,
           GameplayConfig::doppelRollDuration,
           GameplayConfig::doppelRollCooldown),
      parry(GameplayConfig::doppelParryWindow,
            GameplayConfig::doppelParryRecovery,
            GameplayConfig::doppelParryCooldown,
            GameplayConfig::parryStunDuration),
      xpReward(GameplayConfig::doppelBaseXp +
               (std::max(1, level) - 1) * GameplayConfig::doppelXpPerLevel) {
  const auto& stats = GameplayConfig::doppel;
  const int extraLevels = std::max(1, level) - 1;

  const int scaledHp = stats.baseHp + extraLevels * stats.hpPerLevel;
  setMaxHp(scaledHp);
  heal(scaledHp);

  const float scaledSpeed = std::min(
      stats.baseSpeed + extraLevels * stats.speedPerLevel,
      stats.maxSpeed);
  setBaseMoveSpeed(scaledSpeed);

  const int scaledDamage = stats.baseDamage + extraLevels * stats.damagePerLevel;
  setPrimaryAttack(std::make_unique<ArcAttack>(
      scaledDamage,
      GameplayConfig::doppelAttackCooldown,
      stats.attackRange,
      GameplayConfig::doppelAttackAngle,
      GameplayConfig::meleeKnockback));
}

void Doppel::update(float dt, const std::vector<Character*>& worldCharacters) {
  if (!isActive || dt <= 0.0f) return;

  roll.update(*this, dt);
  parry.update(dt);

  Character::update(dt, worldCharacters);
}

void Doppel::update(float dt,
                    Player& player,
                    const std::vector<Character*>& targets,
                    playerAction lastPlayerAction) {
  if (!isActive || dt <= 0.0f) return;

  if (!player.getIsActive()) {
    if (wasSeeingPlayer) {
      brains.resetTracking();
      wasSeeingPlayer = false;
      hasActiveAction = false;
      decisionTimer = 0.0f;
      pendingReward = 0.0;
    }
    if (canAct()) {
      move(QVector2D(0.0f, 0.0f));
    }
    update(dt, targets);
    return;
  }

  const QVector2D toPlayer = player.getPosition() - position;
  const bool seesPlayer = toPlayer.length() <= GameplayConfig::doppelVisionRange;

  if (!seesPlayer) {
    if (wasSeeingPlayer) {
      brains.resetTracking();
      wasSeeingPlayer = false;
      hasActiveAction = false;
      decisionTimer = 0.0f;
      pendingReward = 0.0;
    }

    if (canAct()) {
      move(toPlayer);
    }

    update(dt, targets);
    return;
  }

  wasSeeingPlayer = true;
  decisionTimer -= dt;

  if (decisionTimer <= 0.0f && canAct()) {
    const QVector2D perceptionFacing = getFacingVector();
    egocentricPerception perception = buildPerception(player, targets, lastPlayerAction);
    lastPerception = perception;

    if (hasActiveAction) {
      brains.applyReward(pendingReward, perception);
      pendingReward = 0.0;
    }

    currentAction = brains.update(perception);
    currentActionDir = rotateVectorDegrees(perceptionFacing, currentAction.angle);
    hasActiveAction = true;
    decisionTimer = GameplayConfig::doppelDecisionInterval;

    executeCurrentAction(player, targets);
  } else if (hasActiveAction && canAct() && isMoveAction(currentAction.type)) {
    move(currentActionDir);
  }

  update(dt, targets);
}

bool Doppel::takeDamage(int amount, Character* attacker, bool parryable) {
  if (!isActive || amount <= 0) return false;

  if (parryable && parry.tryDeflect(*this, attacker)) {
    pendingReward += REWARD_PARRY_SUCCESS;
    return false;
  }

  const bool damaged = Character::takeDamage(amount, attacker, parryable);
  if (damaged) {
    pendingReward += REWARD_TAKEN_DAMAGE;

    if (!isActive && hasActiveAction) {
      brains.applyReward(pendingReward + REWARD_DEATH, lastPerception);
      pendingReward = 0.0;
      hasActiveAction = false;
    }
  }

  return damaged;
}

bool Doppel::canAct() const {
  return !roll.isActive() && !parry.isBusy() && Character::canAct();
}

egocentricPerception Doppel::buildPerception(const Player& player,
                                             const std::vector<Character*>& targets,
                                             playerAction lastPlayerAction) const {
  egocentricPerception perception;
  perception.lastPlayerAction = lastPlayerAction;

  const QVector2D selfFacing = getFacingVector();
  const QVector2D playerFacing = player.getFacingVector();
  const QVector2D toPlayer = player.getPosition() - position;

  VisibleEntity playerEntity;
  playerEntity.type = entityType::PLAYER;
  playerEntity.distance = static_cast<double>(toPlayer.length());
  playerEntity.angle = signedAngleDegrees(selfFacing, toPlayer);
  playerEntity.distanceToPlayer = playerEntity.distance;
  playerEntity.angleToPlayer = signedAngleDegrees(playerFacing, -toPlayer);
  perception.visibleEntityArray.push_back(playerEntity);

  for (const Character* other : targets) {
    if (other == nullptr || !other->getIsActive() || other == this || other == &player) {
      continue;
    }

    const QVector2D toOther = other->getPosition() - position;
    const double distToOther = static_cast<double>(toOther.length());
    if (distToOther > GameplayConfig::doppelVisionRange) {
      continue;
    }

    entityType type;
    if (dynamic_cast<const Doppel*>(other) != nullptr) {
      type = entityType::DOPPELGANGER;
    } else if (other->getFaction() == Faction::Enemy) {
      type = entityType::ERRANT;
    } else {
      continue;
    }

    const QVector2D playerToOther = other->getPosition() - player.getPosition();

    VisibleEntity entity;
    entity.type = type;
    entity.distance = distToOther;
    entity.angle = signedAngleDegrees(selfFacing, toOther);
    entity.distanceToPlayer = static_cast<double>(playerToOther.length());
    entity.angleToPlayer = signedAngleDegrees(playerFacing, playerToOther);
    perception.visibleEntityArray.push_back(entity);
  }

  return perception;
}

void Doppel::executeCurrentAction(Player& player, const std::vector<Character*>& targets) {
  switch (currentAction.type) {
    case doppelAction::MOVE_FORWARD:
    case doppelAction::MOVE_AWAY:
    case doppelAction::MOVE_RIGHT:
    case doppelAction::MOVE_LEFT:
      move(currentActionDir);
      break;

    case doppelAction::ROLL_FORWARD:
    case doppelAction::ROLL_AWAY:
    case doppelAction::ROLL_RIGHT:
    case doppelAction::ROLL_LEFT:
      if (!executeRoll(currentActionDir)) {
        pendingReward += REWARD_INVALID_ACTION;
      }
      break;

    case doppelAction::ATTACK: {
      moveVelocity = QVector2D(0.0f, 0.0f);
      const QVector2D toPlayer = player.getPosition() - position;
      const QVector2D dir = toPlayer.isNull() ? getFacingVector() : toPlayer;
      const int playerHpBefore = player.getHp();

      if (attack(dir, targets)) {
        if (player.getHp() < playerHpBefore) {
          pendingReward += REWARD_HIT_PLAYER;
        } else if (!canAct()) {
          pendingReward += REWARD_WAS_PARRIED;
        } else {
          pendingReward += REWARD_MISSED_ATTACK;
        }
      } else {
        pendingReward += REWARD_INVALID_ACTION;
      }
      break;
    }

    case doppelAction::PARRY:
      if (!executeParry()) {
        pendingReward += REWARD_INVALID_ACTION;
      }
      break;
  }
}

bool Doppel::executeRoll(QVector2D dir) {
  if (!isActive || !canAct()) return false;

  if (dir.isNull()) {
    dir = getFacingVector();
  }

  if (physics) {
    dir = physics->filterDirection(dir);
  }

  if (roll.execute(*this, dir)) {
    if (!dir.isNull()) {
      updateFacingDirection(dir);
    }
    return true;
  }

  return false;
}

bool Doppel::executeParry() {
  if (!isActive || !canAct()) return false;

  if (parry.execute()) {
    moveVelocity = QVector2D(0.0f, 0.0f);
    return true;
  }

  return false;
}
