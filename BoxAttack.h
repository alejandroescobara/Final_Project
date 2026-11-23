#pragma once

#include <QVector2D>
#include <vector>

#include "ProjectileAttack.h"

class BoneAttack final : public ProjectileAttack {
  private:
    float upwardBoost;
    int boneCount;
    float spreadAngleDeg;

    Projectile createBone(const Character& attacker, QVector2D velocity) const;

    void onExecute(Character& attacker,
                   QVector2D dir,
                   const std::vector<Character*>& targets) override;

  public:
    BoneAttack(int damage,
               float cooldown,
               float projectileSpeed,
               float projectileRadius,
               float projectileLifetime,
               ProjectileSpawner spawner = nullptr,
               float upwardBoost = 350.0f,
               float gravity = 980.0f,
               int boneCount = 1,
               float spreadAngleDeg = 15.0f,
               float knockbackForce = 0.0f,
               bool parryable = true,
               bool hasGround = false,
               float groundY = 0.0f);

    void setUpwardBoost(float newUpwardBoost);
    void setBoneCount(int newBoneCount);
    void setSpreadAngleDeg(float newSpreadAngleDeg);

    // METODOS TRIVIALES

    float getUpwardBoost() const { return upwardBoost; }
    int getBoneCount() const { return boneCount; }
    float getSpreadAngleDeg() const { return spreadAngleDeg; }
};
