#include "PhysicsMotor.h"

#include <algorithm>
#include <QtMath>

PhysicsMotor::PhysicsMotor(float friction)
            : friction(std::max(0.0f, friction)) {}

void PhysicsMotor::applyExternalFriction(QVector2D& externalVelocity, float dt) const {
  externalVelocity *= std::exp(-friction*dt);

  if (externalVelocity.lengthSquared() < 1.0f) {
    externalVelocity = QVector2D(0.0f, 0.0f);
  }
}

void PhysicsMotor::setFriction(float newFriction) {
  friction = std::max(0.0f, newFriction);
}
