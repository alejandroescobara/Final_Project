#pragma once

#include "PhysicsMotor.h"

class PlanarPhysicsMotor: public PhysicsMotor {
  public:
    explicit PlanarPhysicsMotor(float friction = 10.0f);

    QVector2D filterDirection(QVector2D dir) const override;
    
    void update(QVector2D& position,
                QVector2D moveVelocity,
                QVector2D& externalVelocity,
                float dt) override;
};
