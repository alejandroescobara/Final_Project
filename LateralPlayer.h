#pragma once

#include <QVector2D>

#include "Player.h"

class LateralPhysicsMotor;

class LateralPlayer final : public Player {
  private:
    float jumpSpeed;
    bool blocking = false;

    void updateFacingDirection(QVector2D dir) override;

  public:
    LateralPlayer(QVector2D initialPos,
                  int maxHealth,
                  float baseMoveSpeed,
                  float weight,
                  float jumpSpeed,
                  float groundY,
                  const Roll& rollConfig,
                  const Parry& parryConfig,
                  float gravity = 980.0f,
                  float friction = 10.0f);

    bool jump();
    bool executeParry() override;
    bool canAct() const override;
    bool takeDamage(int amount, Character* attacker = nullptr,
                    bool parryable = true) override;
    bool setBlocking(bool enabled);
    bool isBlocking() const override { return blocking; }
    bool executeRoll(QVector2D dir) override;

    bool isGrounded() const;
    LateralPhysicsMotor* getLateralPhysics() const;

    void setJumpSpeed(float newJumpSpeed);

    // METODOS TRIVIALES

    float getJumpSpeed() const { return jumpSpeed; }
};
