#pragma once

#include "Attack.h"

class BoxAttack final : public Attack {
  private:
    float range;
    float halfWidth;

    void onExecute(Character& attacker,
                   QVector2D direction,
                   const std::vector<Character*>& targets) override;

  public:
    BoxAttack(int damage,
              float cooldown,
              float range,
              float halfWidth,
              float knockbackForce = 0.0f,
              bool parryable = true);
};
