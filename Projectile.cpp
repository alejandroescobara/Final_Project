#include "Projectile.h"

#include "Player.h"

#include <algorithm>

Projectile::Projectile(QVector2D initialPos,
                       QVector2D velocity,
                       int damage,
                       float radius,
                       float lifetime,
                       Faction sourceFaction,
                       float knockbackForce,
                       float gravity,
                       bool parryable,
                       bool hasGround,
                       float groundY)
    : position(initialPos),
      velocity(velocity),
      damage(std::max(0, damage)),
      knockbackForce(std::max(0.0f, knockbackForce)),
      radius(std::max(0.01f, radius)),
      lifetime(std::max(0.01f, lifetime)),
      gravity(std::max(0.0f, gravity)),
      groundY(groundY),
      sourceFaction(sourceFaction),
      parryable(parryable),
      hasGround(hasGround) {}

bool Projectile::canHit(const Character& target) const {
  if (!target.getIsActive()) {
    return false;
  }

  if (sourceFaction == Faction::Neutral || target.getFaction() == Faction::Neutral) {
    return false;
  }

  return sourceFaction != target.getFaction();
}

void Projectile::update(float dt, const std::vector<Character*>& targets) {
  if (!active || dt <= 0.0f) {
    return;
  }

  lifetime -= dt;
  if (lifetime <= 0.0f) {
    lifetime = 0.0f;
    active = false;
    return;
  }

  if (gravity > 0.0f) {
    velocity.setY(velocity.y() + gravity * dt);
  }

  position += velocity * dt;

  const bool hitGround = hasGround && position.y() >= groundY;
  if (hitGround) {
    position.setY(groundY);
  }

  const float hitRadiusSq = radius * radius;
  for (Character* target : targets) {
    if (target == nullptr || !canHit(*target)) {
      continue;
    }

    if (const auto* player = dynamic_cast<const Player*>(target);
        player != nullptr && player->isRolling()) {
      continue;
    }

    if ((target->getPosition() - position).lengthSquared() <= hitRadiusSq) {
      const bool hit = target->takeDamage(damage, nullptr, parryable);

      if (hit && knockbackForce > 0.0f && !velocity.isNull()) {
        target->applyKnockback(velocity, knockbackForce);
      }

      active = false;
      return;
    }
  }

  if (hitGround) {
    active = false;
  }
}
