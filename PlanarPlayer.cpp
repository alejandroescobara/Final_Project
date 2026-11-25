#include "PlanarPlayer.h"

#include <algorithm>
#include <memory>

#include "ArcAttack.h"
#include "GameplayConfig.h"
#include "PlanarPhysicsMotor.h"

PlanarPlayer::PlanarPlayer(QVector2D initialPosition)
    : Player(initialPosition,
             GameplayConfig::planarPlayer.baseHp,
             GameplayConfig::planarPlayer.baseSpeed,
             GameplayConfig::planarPlayer.weight,
             std::make_unique<PlanarPhysicsMotor>(GameplayConfig::movementFriction),
             Roll(GameplayConfig::playerRollDistance / GameplayConfig::playerRollDuration,
                   GameplayConfig::playerRollDuration,
                   GameplayConfig::playerRollCooldown),
             Parry(GameplayConfig::playerParryWindow,
                   GameplayConfig::parryRecovery,
                   GameplayConfig::parryCooldown,
                   GameplayConfig::parryStunDuration)) {
  const auto& stats = GameplayConfig::planarPlayer;

  setPrimaryAttack(std::make_unique<ArcAttack>(
      stats.baseDamage,
      GameplayConfig::playerMeleeCooldown,
      stats.attackRange,
      GameplayConfig::planarPlayerAttackAngle,
      GameplayConfig::meleeKnockback));
}

void PlanarPlayer::levelUp() {
  const auto& stats = GameplayConfig::planarPlayer;
  ++level;

  setMaxHp(getMaxHp() + stats.hpPerLevel);
  heal(stats.hpPerLevel);

  if (getBaseMoveSpeed() < stats.maxSpeed) {
    setBaseMoveSpeed(std::min(getBaseMoveSpeed() + stats.speedPerLevel, stats.maxSpeed));
  }

  if (primaryAttack) {
    primaryAttack->setDamage(primaryAttack->getDamage() + stats.damagePerLevel);
  }
}
