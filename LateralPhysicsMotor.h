#pragma once

#include <QRectF>
#include <utility>
#include <vector>

#include "PhysicsMotor.h"

class LateralPhysicsMotor : public PhysicsMotor {
  private:
    float gravity;
    float groundY;
    float verticalVelocity = 0.0f;
    bool grounded = false;
    std::vector<QRectF> platforms;

  public:
    LateralPhysicsMotor(float groundY, float gravity = 980.0f, float friction = 10.0f);

    QVector2D filterDirection(QVector2D dir) const override;

    void update(QVector2D& position,
                QVector2D moveVelocity,
                QVector2D& externalVelocity,
                float dt) override;

    void reset() override;

    bool jump(float jumpSpeed);

    bool isGrounded() const { return grounded; }
    float getVerticalVelocity() const { return verticalVelocity; }

    void setGroundY(float newGroundY) { groundY = newGroundY; }
    void setPlatforms(std::vector<QRectF> newPlatforms) { platforms = std::move(newPlatforms); }
    const std::vector<QRectF>& getPlatforms() const { return platforms; }
    float getGroundY() const { return groundY; }

    void setGravity(float newGravity);
    float getGravity() const { return gravity; }
};

