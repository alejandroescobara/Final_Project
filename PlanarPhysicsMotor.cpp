#include "PlanarPhysicsMotor.h"

PlanarPhysicsMotor::PlanarPhysicsMotor(float friction)
                  : PhysicsMotor(friction) {}

QVector2D PlanarPhysicsMotor::filterDirection(QVector2D dir) const { return dir; }

void PlanarPhysicsMotor::update(QVector2D& position,
                                QVector2D  moveVelocity,
                                QVector2D& externalVelocity,
                                float dt) {
  position += (moveVelocity + externalVelocity) * dt;
  applyExternalFriction(externalVelocity, dt);
}


