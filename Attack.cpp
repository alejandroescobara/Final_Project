#include "Attack.h"
#include "Character.h"

#include <algorithm>

Attack::Attack(int damage, float cooldown, float knockbackForce, bool parryable)
    : damage(std::max(0, damage)),
      cooldown(std::max(0.01f, cooldown)),
      parryable(parryable),
      force(knockbackForce) {}

void Attack::update(float dt) {
  if (dt <= 0.0f || cooldownTimer <= 0.0f) {
    return;
  }

  cooldownTimer = std::max(0.0f, cooldownTimer - dt);
}

bool Attack::execute(Character& attacker, QVector2D dir, const std::vector<Character*>& targets) {
  if (!attacker.getIsActive() || !isReady() || dir.isNull()) {
    return false;
  }

  std::vector<Character*> validTargets;
  validTargets.reserve(targets.size());
  for (Character* target : targets) {
    if (target != nullptr && isValidTarget(attacker, *target)) {
      validTargets.push_back(target);
    }
  }

  cooldownTimer = cooldown;
  onExecute(attacker, dir.normalized(), validTargets);
  return true;
}


void Attack::setDamage(int newDamage) {
  damage = std::max(0, newDamage);
}

void Attack::setCooldown(float newCooldown) {
  cooldown = std::max(0.01f, newCooldown);
  cooldownTimer = std::min(cooldownTimer, cooldown);
}

bool Attack::isValidTarget(const Character& attacker, const Character& target) const {
  return target.getIsActive() && &target != &attacker && attacker.isHostileTo(target);
}
