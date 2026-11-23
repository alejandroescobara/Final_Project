#include "Silbon.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include "BoneAttack.h"
#include "BoxAttack.h"
#include "GameplayConfig.h"
#include "LateralPhysicsMotor.h"
#include "Tureco.h"

Silbon::Silbon(QVector2D initialPosition, ProjectileAttack::ProjectileSpawner spawner)
    : Character(initialPosition,
                GameplayConfig::silbonHealth,
                GameplayConfig::silbonMoveSpeed,
                GameplayConfig::silbonWeight,
                std::make_unique<LateralPhysicsMotor>(GameplayConfig::levelGroundY,
                                                      GameplayConfig::lateralGravity,
                                                      GameplayConfig::movementFriction),
                Faction::Enemy),
      boneAttack(std::make_unique<BoneAttack>(GameplayConfig::boneDamage,
                                              GameplayConfig::boneCooldown,
                                              GameplayConfig::boneProjectileSpeed,
                                              GameplayConfig::boneProjectileRadius,
                                              GameplayConfig::boneProjectileLifetime,
                                              std::move(spawner),
                                              GameplayConfig::boneProjectileGravity)) {
  setPrimaryAttack(std::make_unique<BoxAttack>(GameplayConfig::silbonMeleeDamage,
                                                GameplayConfig::silbonMeleeCooldown,
                                                GameplayConfig::meleeRange,
                                                GameplayConfig::enemyMeleeHalfWidth,
                                                GameplayConfig::meleeKnockback));
}

Silbon::~Silbon() = default;

void Silbon::update(float dt,
                    const std::vector<Character*>& targets,
                    QVector2D playerPosition,
                    Tureco& tureco) {
  if (!getIsActive() || dt <= 0.0f) return;

  boneAttack->update(dt);
  boneTimer = std::max(0.0f, boneTimer - dt);
  kickTimer = std::max(0.0f, kickTimer - dt);
  attachedWaitTimer = std::max(0.0f, attachedWaitTimer - dt);
  specialAttackSpacingTimer = std::max(0.0f, specialAttackSpacingTimer - dt);

  const bool turecoIsKicked = tureco.isKicked();
  if (turecoWasKicked && !turecoIsKicked) {
    attachedWaitTimer = GameplayConfig::turecoAttachedWait;
  }
  turecoWasKicked = turecoIsKicked;

  const QVector2D difference = playerPosition - getPosition();
  const float distance = std::abs(difference.x());
  if (distance > GameplayConfig::meleeRange * 0.72f) {
    move(QVector2D(difference.x() > 0.0f ? 1.0f : -1.0f, 0.0f));
  } else {
    move(QVector2D());
    attack(QVector2D(difference.x() >= 0.0f ? 1.0f : -1.0f, 0.0f), targets);
  }

  if (specialAttackSpacingTimer <= 0.0f) {
    if (boneTimer <= 0.0f && distance > 130.0f) {
      boneAttack->execute(*this, difference, targets);
      boneTimer = GameplayConfig::boneCooldown;
      specialAttackSpacingTimer = GameplayConfig::specialAttackSpacing;
    } else if (kickTimer <= 0.0f && attachedWaitTimer <= 0.0f && !tureco.isKicked() &&
               tureco.kickAway(getPosition().x(), playerPosition.x())) {
      kickTimer = GameplayConfig::turecoKickCooldown;
      specialAttackSpacingTimer = GameplayConfig::specialAttackSpacing;
    }
  }

  Character::update(dt, targets);
}
