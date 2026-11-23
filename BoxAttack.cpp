#include "BoxAttack.h"

#include "BoneAttack.h"

#include <QtMath>
#include <algorithm>
#include <cmath>

#include "Character.h"

namespace {
  QVector2D rotateVector(QVector2D vec, float angleDeg) {
    const float rad = qDegreesToRadians(angleDeg);
    const float cosA = std::cos(rad);
    const float sinA = std::sin(rad);

    return QVector2D(vec.x() * cosA - vec.y() * sinA,
                     vec.x() * sinA + vec.y() * cosA);
  }
}

BoneAttack::BoneAttack(int damage,
                       float cooldown,
                       float projectileSpeed,
                       float projectileRadius,
                       float projectileLifetime,
                       ProjectileSpawner spawner,
                       float upwardBoost,
                       float gravity,
                       int boneCount,
                       float spreadAngleDeg,
                       float knockbackForce,
                       bool parryable,
                       bool hasGround,
                       float groundY)
    : ProjectileAttack(damage,
                       cooldown,
                       projectileSpeed,
                       projectileRadius,
                       projectileLifetime,
                       std::move(spawner),
                       knockbackForce,
                       gravity,
                       parryable,
                       hasGround,
                       groundY),
      upwardBoost(std::max(0.0f, upwardBoost)),
      boneCount(std::max(1, boneCount)),
      spreadAngleDeg(std::max(0.0f, spreadAngleDeg)) {}

Projectile BoneAttack::createBone(const Character& attacker, QVector2D velocity) const {
  return Projectile(attacker.getPosition(),
                    velocity,
                    getDamage(),
                    getProjectileRadius(),
                    getProjectileLifetime(),
                    attacker.getFaction(),
                    force,
                    getGravity(),
                    isParryable(),
                    usesGround(),
                    getGroundY());
}

void BoneAttack::onExecute(Character& attacker,
                           QVector2D dir,
                           const std::vector<Character*>& /*targets*/) {
  QVector2D baseVelocity = dir * getProjectileSpeed();
  baseVelocity.setY(baseVelocity.y() - upwardBoost);

  if (boneCount == 1) {
    spawnProjectile(createBone(attacker, baseVelocity));
    return;
  }

  for (int i = 0; i < boneCount; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(boneCount - 1);
    const float angleOffset = (-0.5f + t) * spreadAngleDeg;
    const QVector2D boneVelocity = rotateVector(baseVelocity, angleOffset);

    spawnProjectile(createBone(attacker, boneVelocity));
  }
}

void BoneAttack::setUpwardBoost(float newUpwardBoost) {
  upwardBoost = std::max(0.0f, newUpwardBoost);
}

void BoneAttack::setBoneCount(int newBoneCount) {
  boneCount = std::max(1, newBoneCount);
}

void BoneAttack::setSpreadAngleDeg(float newSpreadAngleDeg) {
  spreadAngleDeg = std::max(0.0f, newSpreadAngleDeg);
}
