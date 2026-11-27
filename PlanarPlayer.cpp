#include "PlanarPlayer.h"

#include <algorithm>
#include <memory>

#include <QRandomGenerator>

#include "ArcAttack.h"
#include "GameplayConfig.h"
#include "Item.h"
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
                   GameplayConfig::parryStunDuration)),
      xpToNextLevel(GameplayConfig::planarBaseXpToLevel) {
  const auto& stats = GameplayConfig::planarPlayer;

  setPrimaryAttack(std::make_unique<ArcAttack>(
      stats.baseDamage,
      GameplayConfig::playerMeleeCooldown,
      stats.attackRange,
      GameplayConfig::planarPlayerAttackAngle,
      GameplayConfig::meleeKnockback));
}

bool PlanarPlayer::addXp(int amount) {
  if (!getIsActive() || amount <= 0) {
    return false;
  }

  xp += amount;
  bool leveledUp = false;

  while (xp >= xpToNextLevel) {
    xp -= xpToNextLevel;
    levelUp();
    grantRosaryOnLevelUp();
    leveledUp = true;
  }

  return leveledUp;
}

void PlanarPlayer::levelUp() {
  const auto& stats = GameplayConfig::planarPlayer;
  ++level;
  xpToNextLevel += GameplayConfig::planarXpPerLevel;

  setMaxHp(getMaxHp() + stats.hpPerLevel);
  heal(stats.hpPerLevel);

  if (getBaseMoveSpeed() < stats.maxSpeed) {
    setBaseMoveSpeed(std::min(getBaseMoveSpeed() + stats.speedPerLevel, stats.maxSpeed));
  }

  if (primaryAttack) {
    primaryAttack->setDamage(primaryAttack->getDamage() + stats.damagePerLevel);
  }
}

void PlanarPlayer::grantRosaryOnLevelUp() {
  if (QRandomGenerator::global()->bounded(100) >=
      static_cast<quint32>(GameplayConfig::level1RosaryDropChancePercent)) {
    return;
  }

  for (const auto& item : getEquippedItems()) {
    if (auto* rosary = dynamic_cast<Rosary*>(item.get())) {
      rosary->refreshDuration(GameplayConfig::level1RosaryDuration);
      return;
    }
  }

  equipItem(std::make_unique<Rosary>(GameplayConfig::level1RosaryRegenPerSecond,
                                     GameplayConfig::level1RosaryDuration));
}

float PlanarPlayer::getRosaryRemainingTime() const {
  for (const auto& item : getEquippedItems()) {
    if (const auto* rosary = dynamic_cast<const Rosary*>(item.get())) {
      return rosary->getRemainingDuration();
    }
  }
  return 0.0f;
}
