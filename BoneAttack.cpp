#include "BoneAttack.h"

#include <algorithm>
#include <cmath>

#include "Character.h"
#include "GameplayConfig.h"
#include "LateralPhysicsMotor.h"
#include "LateralPlayer.h"

BoneAttack::BoneAttack(int damage,
                       float cooldown,
                       float projectileSpeed,
                       float projectileRadius,
                       float projectileLifetime,
                       ProjectileSpawner spawner,
                       float gravity,
                       float knockbackForce)
    : ProjectileAttack(damage,
                       cooldown,
                       projectileSpeed,
                       projectileRadius,
                       projectileLifetime,
                       std::move(spawner),
                       knockbackForce,
                       gravity,
                       true,
                       true,
                       GameplayConfig::levelGroundY) {}

void BoneAttack::onExecute(Character& attacker,
                           QVector2D direction,
                           const std::vector<Character*>& targets) {
  const Character* target = nullptr;
  float shortestDistance = 0.0f;
  for (Character* candidate : targets) {
    if (candidate == nullptr || !attacker.isHostileTo(*candidate)) continue;
    const float distance = (candidate->getPosition() - attacker.getPosition()).lengthSquared();
    if (target == nullptr || distance < shortestDistance) {
      target = candidate;
      shortestDistance = distance;
    }
  }

  QVector2D launchPosition = attacker.getPosition();
  launchPosition.setY(launchPosition.y() - GameplayConfig::boneLaunchHeight);
  QVector2D velocity;
  float flightTime = 0.8f;

  if (target != nullptr) {
    flightTime = std::clamp(std::abs(target->getPosition().x() - launchPosition.x()) /
                                getProjectileSpeed(),
                            GameplayConfig::boneMinimumFlightTime,
                            GameplayConfig::boneMaximumFlightTime);

    QVector2D predictedTarget = target->getPosition() + target->getMoveVelocity() * flightTime;
    if (const auto* lateralTarget = dynamic_cast<const LateralPlayer*>(target)) {
      if (const auto* lateralPhysics = lateralTarget->getLateralPhysics()) {
        const float verticalVelocity = lateralPhysics->getVerticalVelocity();
        predictedTarget.setY(predictedTarget.y() + verticalVelocity * flightTime +
                             0.5f * getGravity() * flightTime * flightTime);
        predictedTarget.setY(std::clamp(predictedTarget.y(), 0.0f,
                                        GameplayConfig::levelGroundY));
      }
    }
    velocity.setX((predictedTarget.x() - launchPosition.x()) / flightTime);
    velocity.setY((predictedTarget.y() - launchPosition.y()) / flightTime -
                  0.5f * getGravity() * flightTime);
  } else {
    direction.normalize();
    velocity = direction * getProjectileSpeed();
    velocity.setY(velocity.y() - getGravity() * 0.5f);
  }

  Projectile projectile(launchPosition, velocity, getDamage(), getProjectileRadius(),
                        getProjectileLifetime(), attacker.getFaction(), getForce(),
                        getGravity(), isParryable(), true, GameplayConfig::levelGroundY);
  spawnProjectile(std::move(projectile));
}
