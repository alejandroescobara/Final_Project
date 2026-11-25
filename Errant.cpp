#include "Errant.h"

#include <algorithm>
#include <memory>

#include "GameplayConfig.h"
#include "PlanarPhysicsMotor.h"
#include "Player.h"

Errant::Errant(QVector2D initialPosition, int level)
    : Character(initialPosition,
                GameplayConfig::errant.baseHp,
                GameplayConfig::errant.baseSpeed,
                GameplayConfig::errant.weight,
                std::make_unique<PlanarPhysicsMotor>(GameplayConfig::movementFriction),
                Faction::Enemy),
      contactDamage(GameplayConfig::errant.baseDamage +
                    (std::max(1, level) - 1) * GameplayConfig::errant.damagePerLevel),
      xpReward(GameplayConfig::errantBaseXp +
               (std::max(1, level) - 1) * GameplayConfig::errantXpPerLevel) {
  const auto& stats = GameplayConfig::errant;
  const int extraLevels = std::max(1, level) - 1;

  const int scaledHp = stats.baseHp + extraLevels * stats.hpPerLevel;
  setMaxHp(scaledHp);
  heal(scaledHp);

  const float scaledSpeed = std::min(
      stats.baseSpeed + extraLevels * stats.speedPerLevel,
      stats.maxSpeed);
  setBaseMoveSpeed(scaledSpeed);
}

void Errant::update(float dt,
                    Player& player,
                    const std::vector<Character*>& targets) {
  if (!isActive || dt <= 0.0f) return;

  if (player.getIsActive()) {
    move(player.getPosition() - position);
  } else {
    move(QVector2D(0.0f, 0.0f));
  }

  Character::update(dt, targets);
  if (!isActive || !player.getIsActive() || !isHostileTo(player)) return;

  const QVector2D toPlayer = player.getPosition() - position;
  if (toPlayer.length() <= GameplayConfig::errant.attackRange) {
    player.takeDamage(contactDamage, this, false);
    hitPlayer = true;
    isActive = false;
    moveVelocity = QVector2D(0.0f, 0.0f);
    externalVelocity = QVector2D(0.0f, 0.0f);
  }
}
