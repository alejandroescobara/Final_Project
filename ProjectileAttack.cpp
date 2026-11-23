#include "ProjectileAttack.h"

#include <algorithm>

#include "Character.h"

ProjectileAttack::ProjectileAttack(int damage,
                                   float cooldown,
                                   float projectileSpeed,
                                   float projectileRadius,
                                   float projectileLifetime,
                                   ProjectileSpawner spawner,
                                   float knockbackForce,
                                   float gravity,
                                   bool parryable,
                                   bool hasGround,
                                   float groundY)
    : Attack(damage, cooldown, knockbackForce, parryable),
      projectileSpeed(std::max(0.01f, projectileSpeed)),
      projectileRadius(std::max(0.01f, projectileRadius)),
      projectileLifetime(std::max(0.01f, projectileLifetime)),
      gravity(std::max(0.0f, gravity)),
      hasGround(hasGround),
      groundY(groundY),
      spawner(std::move(spawner)) {}

Projectile ProjectileAttack::createProjectile(const Character& attacker, QVector2D dir) const {
  return Projectile(attacker.getPosition(),
                    dir * projectileSpeed,
                    getDamage(),
                    projectileRadius,
                    projectileLifetime,
                    attacker.getFaction(),
                    force,
                    gravity,
                    isParryable(),
                    hasGround,
                    groundY);
}

void ProjectileAttack::spawnProjectile(Projectile projectile) const {
  if (spawner) {
    spawner(std::move(projectile));
  }
}

void ProjectileAttack::onExecute(Character& attacker,
                                 QVector2D dir,
                                 const std::vector<Character*>& /*targets*/) {
  spawnProjectile(createProjectile(attacker, dir));
}

void ProjectileAttack::setProjectileSpeed(float newSpeed) {
  projectileSpeed = std::max(0.01f, newSpeed);
}

void ProjectileAttack::setProjectileRadius(float newRadius) {
  projectileRadius = std::max(0.01f, newRadius);
}

void ProjectileAttack::setProjectileLifetime(float newLifetime) {
  projectileLifetime = std::max(0.01f, newLifetime);
}

void ProjectileAttack::setGravity(float newGravity) {
  gravity = std::max(0.0f, newGravity);
}
