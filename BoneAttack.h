#pragma once

#include "ProjectileAttack.h"

class BoneAttack final : public ProjectileAttack {
  private:
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
               float knockbackForce = 0.0f);
};
