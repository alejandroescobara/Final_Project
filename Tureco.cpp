#include "Tureco.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include "BoxAttack.h"
#include "GameplayConfig.h"
#include "LateralPhysicsMotor.h"
#include "Player.h"
#include "Silbon.h"

Tureco::Tureco(QVector2D initialPosition, int damage)
    : Character(initialPosition,
                GameplayConfig::turecoHealth,
                0.0f,
                GameplayConfig::enemyWeight,
                std::make_unique<LateralPhysicsMotor>(GameplayConfig::levelGroundY,
                                                      GameplayConfig::lateralGravity,
                                                      GameplayConfig::movementFriction),
                Faction::Enemy) {
  setPrimaryAttack(std::make_unique<BoxAttack>(damage,
                                                GameplayConfig::turecoMeleeCooldown,
                                                GameplayConfig::turecoMeleeRange,
                                                GameplayConfig::turecoMeleeHalfWidth,
                                                GameplayConfig::meleeKnockback));
}

void Tureco::updatePhysics(float dt) {
  if (motionState == MotionState::Attached) {
    setMoveVelocity(QVector2D());
    setExternalVelocity(QVector2D());
  }
  Character::updatePhysics(dt);
}

bool Tureco::kickAway(float silbonX, float playerX) {
  if (!getIsActive() || motionState != MotionState::Attached || kickCooldown > 0.0f) return false;
  velocityX = playerX >= silbonX ? GameplayConfig::turecoKickSpeed
                                 : -GameplayConfig::turecoKickSpeed;
  motionState = MotionState::Outbound;
  playerContactLatched = false;
  kickCooldown = GameplayConfig::turecoKickCooldown;
  return true;
}

void Tureco::update(float dt,
                    Silbon& silbon,
                    Player& player,
                    const std::vector<Character*>& targets) {
  if (!getIsActive() || dt <= 0.0f) return;
  if (kickCooldown > 0.0f) kickCooldown = std::max(0.0f, kickCooldown - dt);

  const float oldX = getPosition().x();
  float nextX = oldX;

  if (motionState == MotionState::Attached) {
    const float side = getPosition().x() >= silbon.getPosition().x() ? 1.0f : -1.0f;
    nextX = silbon.getPosition().x() + side * GameplayConfig::turecoAttachedOffset;
  } else {
    if (motionState == MotionState::Outbound) {
      const float drag = GameplayConfig::movementFriction * 42.0f * dt;
      if (std::abs(velocityX) <= drag) {
        velocityX = 0.0f;
        motionState = MotionState::Returning;
      } else {
        velocityX -= std::copysign(drag, velocityX);
      }
    } else {
      const float direction = silbon.getPosition().x() < oldX ? -1.0f : 1.0f;
      velocityX += direction * GameplayConfig::turecoReturnAcceleration * dt;
    }

    nextX += velocityX * dt;
    const QVector2D playerPosition = player.getPosition();
    const float playerX = playerPosition.x();
    const bool playerIsAtTurecoHeight =
        std::abs(playerPosition.y() - getPosition().y()) <=
        GameplayConfig::turecoMeleeHalfWidth;
    const bool crossedPlayer = playerIsAtTurecoHeight &&
                               std::min(oldX, nextX) <= playerX + 42.0f &&
                               std::max(oldX, nextX) >= playerX - 42.0f;
    if (crossedPlayer && !playerContactLatched) {
      if (!player.isRolling()) {
        if (player.isParrying()) {
          const float towardTureco = getPosition().x() - playerX;
          const float facing = player.getFacingVector().x();
          if (facing * towardTureco > 0.0f) {
            const float towardSilbon = silbon.getPosition().x() - oldX;
            velocityX = (towardSilbon >= 0.0f ? 1.0f : -1.0f) * std::abs(velocityX);
            motionState = MotionState::Returning;
            nextX = playerX + (oldX < playerX ? -42.0f : 42.0f);
          }
        } else {
          attack(QVector2D(playerX >= oldX ? 1.0f : -1.0f, 0.0f), targets);
        }
      }
    }
    playerContactLatched = crossedPlayer;

    if (motionState == MotionState::Returning &&
        std::abs(nextX - silbon.getPosition().x()) <= GameplayConfig::turecoAttachedOffset) {
      const float silbonVelocityX = silbon.getMoveVelocity().x() +
                                    silbon.getExternalVelocity().x();
      const float totalMass = getWeight() + silbon.getWeight();
      const float sharedVelocity = totalMass > 0.0f
          ? (getWeight() * velocityX + silbon.getWeight() * silbonVelocityX) / totalMass
          : 0.0f;
      const float side = oldX >= silbon.getPosition().x() ? 1.0f : -1.0f;
      nextX = silbon.getPosition().x() + side * GameplayConfig::turecoAttachedOffset;

      QVector2D silbonExternalVelocity = silbon.getExternalVelocity();
      silbonExternalVelocity.setX(sharedVelocity - silbon.getMoveVelocity().x());
      silbon.setExternalVelocity(silbonExternalVelocity);

      motionState = MotionState::Attached;
      velocityX = 0.0f;
    }
  }

  setPosition(QVector2D(nextX, getPosition().y()));
  Character::update(dt, targets);
}
