#include "ArcAttack.h"

#include <QtMath>
#include <algorithm>
#include <cmath>

#include "Character.h"

ArcAttack::ArcAttack(int damage,
                     float cooldown,
                     float range,
                     float arcAngleDegrees,
                     float knockbackForce,
                     bool parryable)
    : Attack(damage, cooldown, knockbackForce, parryable),
      range(std::max(1.0f, range)),
      arcAngleDegrees(std::clamp(arcAngleDegrees, 1.0f, 360.0f)),
      minDot(std::cos(qDegreesToRadians(this->arcAngleDegrees * 0.5f))) {}

void ArcAttack::onExecute(Character& attacker,
                          QVector2D direction,
                          const std::vector<Character*>& targets) {
  direction.normalize();
  const QVector2D origin = attacker.getPosition();
  const float rangeSq = range * range;

  for (Character* target : targets) {
    if (target == nullptr) continue;

    const QVector2D offset = target->getPosition() - origin;
    if (offset.lengthSquared() > rangeSq) continue;

    const bool insideArc =
        arcAngleDegrees >= 360.0f ||
        offset.isNull() ||
        QVector2D::dotProduct(offset.normalized(), direction) >= minDot;

    if (insideArc) {
      const bool hit = target->takeDamage(getDamage(), &attacker, isParryable());
      if (hit && getForce() > 0.0f) {
        const QVector2D knockbackDir = offset.isNull() ? direction : offset.normalized();
        target->applyKnockback(knockbackDir, getForce());
      }
    }
  }
}
