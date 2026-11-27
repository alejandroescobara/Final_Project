#include "BoneAttack.h"

#include <algorithm>
#include <cmath>

#include <QRectF>

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
        const float currentY = target->getPosition().y();
        const float predictedY = currentY + verticalVelocity * flightTime +
                                 0.5f * lateralPhysics->getGravity() *
                                     flightTime * flightTime;
        float landingY = GameplayConfig::levelGroundY;
        for (const QRectF& platform : lateralPhysics->getPlatforms()) {
          const bool canLandOnPlatform = currentY <= platform.top() &&
                                        predictedY >= platform.top() &&
                                        predictedTarget.x() >= platform.left() &&
                                        predictedTarget.x() <= platform.right();
          if (canLandOnPlatform && platform.top() < landingY) {
            landingY = static_cast<float>(platform.top());
          }
        }
        predictedTarget.setY(std::clamp(predictedY, 0.0f, landingY));
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
