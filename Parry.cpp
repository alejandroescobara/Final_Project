#include "Parry.h"

#include <QVector2D>
#include <algorithm>
#include <memory>

#include "Character.h"
#include "StatusEffect.h"

Parry::Parry(float activeWindow,
             float recoveryDuration,
             float cooldown,
             float stunDuration,
             float deflectKnockback)
    : activeWindow(std::max(0.01f, activeWindow)),
      recoveryDuration(std::max(0.0f, recoveryDuration)),
      cooldown(std::max(0.0f, cooldown)),
      stunDuration(std::max(0.0f, stunDuration)),
      deflectKnockback(std::max(0.0f, deflectKnockback)) {}

bool Parry::execute() {
  if (!isReady()) {
    return false;
  }

  activeTimer = activeWindow;
  recoveryTimer = recoveryDuration;
  cooldownTimer = cooldown;
  return true;
}

void Parry::update(float dt) {
  if (dt <= 0.0f) return;

  if (activeTimer > 0.0f) {
    if (dt < activeTimer) {
      activeTimer -= dt;
    } else {
      const float leftover = dt - activeTimer;
      activeTimer = 0.0f;
      recoveryTimer = std::max(0.0f, recoveryTimer - leftover);
    }
  } else if (recoveryTimer > 0.0f) {
    recoveryTimer = std::max(0.0f, recoveryTimer - dt);
  }

  if (cooldownTimer > 0.0f) {
    cooldownTimer = std::max(0.0f, cooldownTimer - dt);
  }
}

bool Parry::tryDeflect(Character& owner, Character* attacker) {
  if (!isActive()) {
    return false;
  }

  activeTimer = 0.0f;
  recoveryTimer = 0.0f;
  cooldownTimer = 0.0f; // Cancela el recovery y permite encadenar otro parry tras bloquear con éxito

  if (attacker != nullptr && attacker != &owner) {
    if (stunDuration > 0.0f) {
      attacker->addStatusEffect(std::make_unique<StunEffect>(stunDuration));
    }

    const QVector2D pushDir = attacker->getPosition() - owner.getPosition();
    if (!pushDir.isNull() && deflectKnockback > 0.0f) {
      attacker->applyKnockback(pushDir, deflectKnockback);
    }
  }

  return true;
}
