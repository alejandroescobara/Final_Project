#include "LateralPhysicsMotor.h"

#include <algorithm>

LateralPhysicsMotor::LateralPhysicsMotor(float groundY, float gravity, float friction)
                   : PhysicsMotor(friction),
                     gravity(std::max(0.0f, gravity)),
                     groundY(groundY) {}

QVector2D LateralPhysicsMotor::filterDirection(QVector2D dir) const {
  return QVector2D(dir.x(), 0.0f);
}

void LateralPhysicsMotor::update(QVector2D& position,
                                 QVector2D moveVelocity,
                                 QVector2D& externalVelocity,
                                 float dt) {
  verticalVelocity += gravity * dt;

  const QVector2D totalVelocity = moveVelocity + externalVelocity + QVector2D(0.0f, verticalVelocity);
  position += totalVelocity * dt;

  if (position.y() >= groundY) {
    position.setY(groundY);
    verticalVelocity = 0.0f;
    if (externalVelocity.y() > 0.0f) {
      externalVelocity.setY(0.0f);
    }
    grounded = true;
  }
  else {
    grounded = false;
  }

  applyExternalFriction(externalVelocity, dt);
}

void LateralPhysicsMotor::reset() {
  verticalVelocity = 0.0f;
  grounded = false;
}

bool LateralPhysicsMotor::jump(float jumpSpeed) {
  if (!grounded || jumpSpeed <= 0.0f) return false;

  verticalVelocity = -jumpSpeed;
  grounded = false;
  return true;
}

void LateralPhysicsMotor::setGravity(float newGravity) {
  gravity = std::max(0.0f, newGravity);
}
