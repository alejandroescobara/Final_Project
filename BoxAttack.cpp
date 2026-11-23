#include "BoxAttack.h"

#include <algorithm>
#include <cmath>

#include "Character.h"

BoxAttack::BoxAttack(int damage,
                     float cooldown,
                     float range,
                     float halfWidth,
                     float knockbackForce,
                     bool parryable)
    : Attack(damage, cooldown, knockbackForce, parryable),
      range(std::max(1.0f, range)),
      halfWidth(std::max(1.0f, halfWidth)) {}

void BoxAttack::onExecute(Character& attacker,
                          QVector2D direction,
                          const std::vector<Character*>& targets) {
  direction.normalize();
  const QVector2D side(-direction.y(), direction.x());
  const QVector2D origin = attacker.getPosition();

  for (Character* target : targets) {
    if (target == nullptr) continue;
    const QVector2D offset = target->getPosition() - origin;
    const float forward = QVector2D::dotProduct(offset, direction);
    const float lateral = std::abs(QVector2D::dotProduct(offset, side));

    if (forward >= 0.0f && forward <= range && lateral <= halfWidth) {
      const bool hit = target->takeDamage(getDamage(), &attacker, isParryable());
      if (hit && getForce() > 0.0f) {
        target->applyKnockback(direction, getForce());
      }
    }
  }
}
