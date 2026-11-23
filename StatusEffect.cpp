#include "StatusEffect.h"

#include <algorithm>
#include <cmath>

// <| DEFAULT |>

StatusEffect::StatusEffect(float duration)
    : remainingDuration(std::max(0.0f, duration)) {}

void StatusEffect::update(float dt) {
  if (dt > 0.0f) {
    remainingDuration = std::max(0.0f, remainingDuration - dt);
  }
}

// <| DRUNK |>

DrunkEffect::DrunkEffect(float duration, float turnResponsiveness)
    : StatusEffect(duration),
      responsiveness(std::max(0.5f, turnResponsiveness)) {}

void DrunkEffect::onTick(Character& /*target*/, float dt, const std::vector<Character*>& /*worldCharacters*/) {
  if (dt > 0.0f) {
    lastDt = dt;
  }
}

void DrunkEffect::modifyMoveDirection(QVector2D& dir) {
  const QVector2D targetDir = dir.isNull() ? QVector2D(0.0f, 0.0f) : dir.normalized();

  // Interpolación exponencial independiente de los FPS
  const float alpha = 1.0f - std::exp(-responsiveness * lastDt);
  currentDir += (targetDir - currentDir) * alpha;

  if (currentDir.lengthSquared() < 0.001f) {
    currentDir = QVector2D(0.0f, 0.0f);
  }

  dir = currentDir;
}
