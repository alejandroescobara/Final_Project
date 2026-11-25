#pragma once

#include "Attack.h"

class ArcAttack final : public Attack {
  private:
    float range;
    float arcAngleDegrees;
    float minDot;

    void onExecute(Character& attacker,
                   QVector2D direction,
                   const std::vector<Character*>& targets) override;

  public:
    ArcAttack(int damage,
              float cooldown,
              float range,
              float arcAngleDegrees,
              float knockbackForce = 0.0f,
              bool parryable = true);

    // METODOS TRIVIALES

    float getRange() const { return range; }
    float getArcAngle() const { return arcAngleDegrees; }
};
