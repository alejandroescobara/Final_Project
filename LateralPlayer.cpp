#include "LateralPlayer.h"

#include <algorithm>
#include <memory>

#include "LateralPhysicsMotor.h"

LateralPlayer::LateralPlayer(QVector2D initialPos,
                             int maxHealth,
                             float baseMoveSpeed,
                             float weight,
                             float jumpSpeed,
                             float groundY,
                             const Roll& rollConfig,
                             const Parry& parryConfig,
                             float gravity,
                             float friction)
    : Player(initialPos,
             maxHealth,
             baseMoveSpeed,
             weight,
             std::make_unique<LateralPhysicsMotor>(groundY, gravity, friction),
             rollConfig,
             parryConfig),
      jumpSpeed(std::max(0.0f, jumpSpeed)) {
  facingDirection = Direction::Right;
}
void LateralPlayer::updateFacingDirection(QVector2D dir) {
  if (dir.x() > 0.0f) {
    facingDirection = Direction::Right;
  } else if (dir.x() < 0.0f) {
    facingDirection = Direction::Left;
  }
}

bool LateralPlayer::jump() {
  if (!isActive || !canAct()) {
    return false;
  }

  if (auto* lateralPhysics = getLateralPhysics()) {
    return lateralPhysics->jump(jumpSpeed);
  }

  return false;
}

bool LateralPlayer::executeParry() {
  if (!isGrounded()) {
    return false;
  }

  return Player::executeParry();
}

bool LateralPlayer::executeRoll(QVector2D dir) {
  if (!isGrounded()) return false;
  return Player::executeRoll(dir);
}

bool LateralPlayer::isGrounded() const {
  if (const auto* lateralPhysics = getLateralPhysics()) {
    return lateralPhysics->isGrounded();
  }

  return false;
}

LateralPhysicsMotor* LateralPlayer::getLateralPhysics() const {
  return dynamic_cast<LateralPhysicsMotor*>(physics.get());
}

void LateralPlayer::setJumpSpeed(float newJumpSpeed) {
  jumpSpeed = std::max(0.0f, newJumpSpeed);
}
