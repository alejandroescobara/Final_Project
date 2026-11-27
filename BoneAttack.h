#pragma once

#include "ProjectileAttack.h"

class BoneAttack final : public ProjectileAttack {
  private:
    int projectileCount;
    float spreadAngleDegrees;

    void onExecute(Character& attacker,
                   QVector2D direction,
                   const std::vector<Character*>& targets) override;

  public:
    BoneAttack(int damage,
               float cooldown,
               float projectileSpeed,
               float projectileRadius,
               float projectileLifetime,
               ProjectileSpawner spawner,
               float gravity,
               float knockbackForce = 0.0f,
               int projectileCount = 1,
               float spreadAngleDegrees = 10.0f);
};
