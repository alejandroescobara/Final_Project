#pragma once

#include <QVector2D>

class PhysicsMotor {
  protected:
    float friction;

  void applyExternalFriction(QVector2D& externalVelocity, float dt) const;

  public:
    explicit PhysicsMotor(float friction = 10.0f);
    virtual ~PhysicsMotor() = default;

    virtual QVector2D filterDirection(QVector2D dir) const = 0;
    virtual void update(QVector2D& position,
                        QVector2D  moveVelocity,
                        QVector2D& externalVelocity,
                        float dt) = 0;

    virtual void reset() {}

    void setFriction(float newFriction);
    float getFriction() const { return friction; }
};
