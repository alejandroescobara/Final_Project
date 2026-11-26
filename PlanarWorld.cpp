#include "PlanarWorld.h"

#include <QtMath>
#include <algorithm>
#include <cmath>
#include <filesystem>

#include "GameplayConfig.h"
#include "StatusEffect.h"

namespace {
  constexpr float ACTION_LATCH_DURATION = 0.35f;

  double signedAngleDegrees(QVector2D from, QVector2D to) {
    if (from.isNull() || to.isNull()) return 0.0;

    const double dot = static_cast<double>(from.x()) * to.x() +
                       static_cast<double>(from.y()) * to.y();
    const double cross = static_cast<double>(from.x()) * to.y() -
                         static_cast<double>(from.y()) * to.x();
    return qRadiansToDegrees(std::atan2(cross, dot));
  }
}  // namespace

PlanarWorld::PlanarWorld() {
  sharedAiKnowledge.load();
  reset();
}

void PlanarWorld::reset() {
  const QVector2D startPos(GameplayConfig::level1MapWidth * 0.5f,
                           GameplayConfig::level1MapHeight * 0.85f);

  player = std::make_unique<PlanarPlayer>(startPos);
  player->addStatusEffect(std::make_unique<DrunkEffect>(
      GameplayConfig::level1DrunkDuration,
      GameplayConfig::level1DrunkResponsiveness));

  errants.clear();
  doppels.clear();
  spawner.reset();

  state = PlanarWorldState::Playing;
  elapsedTime = 0.0f;
  drunkTimer = GameplayConfig::level1DrunkDuration;

  doppelsDefeated = 0;
  bossPortalUnlocked = false;

  latchedPlayerAction = playerAction::INACTIVE;
  actionLatchTimer = 0.0f;
}

void PlanarWorld::update(float dt, const PlanarInput& input) {
  if (state != PlanarWorldState::Playing || dt <= 0.0f) return;

  elapsedTime += dt;
  if (drunkTimer > 0.0f) {
    drunkTimer = std::max(0.0f, drunkTimer - dt);
  }

  spawner.update(dt,
                 player->getPosition(),
                 isPlayerDrunk(),
                 doppelsDefeated,
                 sharedAiKnowledge,
                 errants,
                 doppels);

  std::vector<Character*> targets = buildWorldTargets();
  const playerAction observedPlayerAction = processPlayerInput(dt, input, targets);

  player->update(dt, targets);
  clampToArena(*player);

  for (auto& errant : errants) {
    errant->update(dt, *player, targets);
    clampToArena(*errant);
  }

  for (auto& doppel : doppels) {
    doppel->update(dt, *player, targets, observedPlayerAction);
    clampToArena(*doppel);
  }

  handleDeathsAndProgression();

  if (!player->getIsActive()) {
    saveAllKnowledge();
    state = PlanarWorldState::GameOver;
    return;
  }

  checkBossPortal();
}

std::vector<Character*> PlanarWorld::buildWorldTargets() const {
  std::vector<Character*> targets;
  targets.reserve(1 + errants.size() + doppels.size());

  if (player && player->getIsActive()) {
    targets.push_back(player.get());
  }
  for (const auto& errant : errants) {
    if (errant->getIsActive()) {
      targets.push_back(errant.get());
    }
  }
  for (const auto& doppel : doppels) {
    if (doppel->getIsActive()) {
      targets.push_back(doppel.get());
    }
  }

  return targets;
}

playerAction PlanarWorld::processPlayerInput(float dt,
                                             const PlanarInput& input,
                                             const std::vector<Character*>& targets) {
  if (actionLatchTimer > 0.0f) {
    actionLatchTimer = std::max(0.0f, actionLatchTimer - dt);
  }

  // Se llama siempre (incluso con vector nulo) para que DrunkEffect frene la inercia
  player->move(input.moveDirection);

  const QVector2D actionDir = !input.aimDirection.isNull()
                                  ? input.aimDirection
                                  : (!input.moveDirection.isNull()
                                         ? input.moveDirection
                                         : player->getFacingVector());

  if (input.parryPressed && player->executeParry()) {
    latchedPlayerAction = playerAction::PARRY;
    actionLatchTimer = ACTION_LATCH_DURATION;
    return latchedPlayerAction;
  }

  if (input.rollPressed && player->executeRoll(input.moveDirection)) {
    const QVector2D rollDir = input.moveDirection.isNull()
                                  ? player->getFacingVector()
                                  : input.moveDirection;
    latchedPlayerAction = classifyDirectionAction(
        rollDir,
        playerAction::ROLL_FORWARD,
        playerAction::ROLL_AWAY,
        playerAction::ROLL_RIGHT,
        playerAction::ROLL_LEFT);
    actionLatchTimer = ACTION_LATCH_DURATION;
    return latchedPlayerAction;
  }

  if (input.attackPressed && player->attack(actionDir, targets)) {
    latchedPlayerAction = classifyDirectionAction(
        actionDir,
        playerAction::ATTACK_FORWARD,
        playerAction::ATTACK_AWAY,
        playerAction::ATTACK_RIGHT,
        playerAction::ATTACK_LEFT);
    actionLatchTimer = ACTION_LATCH_DURATION;
    return latchedPlayerAction;
  }

  if (actionLatchTimer > 0.0f) {
    return latchedPlayerAction;
  }

  if (!input.moveDirection.isNull()) {
    return classifyDirectionAction(
        input.moveDirection,
        playerAction::MOVE_FORWARD,
        playerAction::MOVE_AWAY,
        playerAction::MOVE_RIGHT,
        playerAction::MOVE_LEFT);
  }

  return playerAction::INACTIVE;
}

QVector2D PlanarWorld::getReferenceDirectionForPlayer() const {
  const Character* nearestThreat = nullptr;
  float minDistSq = 1e18f;

  for (const auto& doppel : doppels) {
    if (!doppel->getIsActive()) continue;
    const float distSq = (doppel->getPosition() - player->getPosition()).lengthSquared();
    if (distSq < minDistSq) {
      minDistSq = distSq;
      nearestThreat = doppel.get();
    }
  }

  if (nearestThreat == nullptr) {
    for (const auto& errant : errants) {
      if (!errant->getIsActive()) continue;
      const float distSq = (errant->getPosition() - player->getPosition()).lengthSquared();
      if (distSq < minDistSq) {
        minDistSq = distSq;
        nearestThreat = errant.get();
      }
    }
  }

  if (nearestThreat != nullptr) {
    const QVector2D toThreat = nearestThreat->getPosition() - player->getPosition();
    if (!toThreat.isNull()) {
      return toThreat.normalized();
    }
  }

  return player->getFacingVector();
}

playerAction PlanarWorld::classifyDirectionAction(QVector2D dir,
                                                  playerAction forward,
                                                  playerAction away,
                                                  playerAction right,
                                                  playerAction left) const {
  const double angle = signedAngleDegrees(getReferenceDirectionForPlayer(), dir);
  const double absAngle = std::abs(angle);

  if (absAngle <= 45.0) return forward;
  if (absAngle >= 135.0) return away;
  return angle > 0.0 ? right : left;
}

void PlanarWorld::handleDeathsAndProgression() {
  bool playerLeveledUp = false;
  bool anyDoppelDefeated = false;

  for (const auto& errant : errants) {
    if (!errant->getIsActive() && !errant->didHitPlayer()) {
      if (player->addXp(errant->getXpReward())) {
        playerLeveledUp = true;
      }
    }
  }

  for (const auto& doppel : doppels) {
    if (!doppel->getIsActive()) {
      ++doppelsDefeated;
      anyDoppelDefeated = true;

      if (player->addXp(doppel->getXpReward())) {
        playerLeveledUp = true;
      }
    }
  }

  std::erase_if(errants, [](const auto& errant) { return !errant->getIsActive(); });
  std::erase_if(doppels, [](const auto& doppel) { return !doppel->getIsActive(); });

  if (doppelsDefeated >= GameplayConfig::level1DoppelsForBoss) {
    bossPortalUnlocked = true;
  }

  if (playerLeveledUp) {
    sharedAiKnowledge.notifyPlayerUpgrade();
  }

  if (anyDoppelDefeated) {
    saveAllKnowledge();
  }
}

void PlanarWorld::clampToArena(Character& character) const {
  if (!character.getIsActive()) return;

  const float margin = GameplayConfig::level1BoundaryMargin;
  const QVector2D pos = character.getPosition();

  const float clampedX = std::clamp(pos.x(), margin, GameplayConfig::level1MapWidth - margin);
  const float clampedY = std::clamp(pos.y(), margin, GameplayConfig::level1MapHeight - margin);
  character.setPosition(QVector2D(clampedX, clampedY));
}

void PlanarWorld::checkBossPortal() {
  if (!bossPortalUnlocked || !player->getIsActive()) return;

  if (getBossPortalRect().contains(player->getPosition().toPointF())) {
    saveAllKnowledge();
    state = PlanarWorldState::ReadyForBossFight;
  }
}

QRectF PlanarWorld::getBossPortalRect() const {
  return QRectF(0.0,
                0.0,
                GameplayConfig::level1MapWidth,
                GameplayConfig::level1BoundaryMargin + GameplayConfig::level1PortalTriggerDepth);
}

void PlanarWorld::saveAllKnowledge() const {
  std::error_code ec;
  const std::filesystem::path qDir = std::filesystem::path(DEFAULT_QTABLE_PATH).parent_path();
  if (!qDir.empty()) {
    std::filesystem::create_directories(qDir, ec);
  }
  const std::filesystem::path mDir = std::filesystem::path(DEFAULT_MARKOV_PATH).parent_path();
  if (!mDir.empty() && mDir != qDir) {
    std::filesystem::create_directories(mDir, ec);
  }
  sharedAiKnowledge.save();
}
