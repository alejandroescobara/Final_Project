#include "Roll.h"

#include <algorithm>
#include <memory>

#include "Character.h"
#include "StatusEffect.h"

Roll::Roll(float speed, float duration, float cooldown, float invulnerabilityDuration)
    : speed(std::max(0.0f, speed)),
      duration(std::max(0.01f, duration)),
      cooldown(std::max(0.0f, cooldown)),
      invulnerabilityDuration(invulnerabilityDuration < 0.0f ? this->duration : invulnerabilityDuration) {}

bool Roll::execute(Character& owner, QVector2D dir) {
  if (!isReady() || dir.isNull()) {
    return false;
  }

  rollDirection = dir.normalized();
  activeTimer = duration;
  cooldownTimer = cooldown;

  owner.setMoveVelocity(rollDirection * speed);

  if (invulnerabilityDuration > 0.0f) {
    owner.addStatusEffect(std::make_unique<InvulnerabilityEffect>(invulnerabilityDuration));
  }

  return true;
}

void Roll::update(Character& owner, float dt) {
  if (dt <= 0.0f) return;

  if (cooldownTimer > 0.0f) {
    cooldownTimer = std::max(0.0f, cooldownTimer - dt);
  }

  if (activeTimer > 0.0f) {
    activeTimer -= dt;
    if (activeTimer > 0.0f) {
      owner.setMoveVelocity(rollDirection * speed);
    } else {
      activeTimer = 0.0f;
      owner.setMoveVelocity(QVector2D(0.0f, 0.0f));
    }
  }
}
