#pragma once

#include "Character.h"

class Player;
class Silbon;

class Tureco final : public Character {
  private:
    enum class MotionState { Attached, Outbound, Returning };

    MotionState motionState = MotionState::Attached;
    float velocityX = 0.0f;
    bool playerContactLatched = false;
    float kickCooldown = 0.0f;

    void updatePhysics(float dt) override;

  public:
    explicit Tureco(QVector2D initialPosition);

    void update(float dt,
                Silbon& silbon,
                Player& player,
                const std::vector<Character*>& targets);
    bool kickAway(float silbonX, float playerX);
    bool isKicked() const { return motionState != MotionState::Attached; }
};
