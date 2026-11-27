#include "Projectile.h"

#include "Player.h"

#include <algorithm>
#include <cmath>

#include <QRectF>
#include <QVector2D>

namespace {
bool segmentIntersectsRect(const QVector2D& start, const QVector2D& end, const QRectF& rect) {
  const float dx = end.x() - start.x();
  const float dy = end.y() - start.y();
  float minT = 0.0f;
  float maxT = 1.0f;
  const auto clip = [&minT, &maxT](float p, float q) {
    if (std::abs(p) < 0.0001f) return q >= 0.0f;
    const float t = q / p;
    if (p < 0.0f) minT = std::max(minT, t);
    else maxT = std::min(maxT, t);
    return minT <= maxT;
  };
  return clip(-dx, start.x() - static_cast<float>(rect.left())) &&
         clip(dx, static_cast<float>(rect.right()) - start.x()) &&
         clip(-dy, start.y() - static_cast<float>(rect.top())) &&
         clip(dy, static_cast<float>(rect.bottom()) - start.y());
}
}  // namespace

Projectile::Projectile(QVector2D initialPos,
                       QVector2D velocity,
                       int damage,
                       float radius,
                       float lifetime,
                       Faction sourceFaction,
                       float knockbackForce,
                       float gravity,
                       bool parryable,
                       bool hasGround,
                       float groundY)
    : position(initialPos),
      velocity(velocity),
      damage(std::max(0, damage)),
      knockbackForce(std::max(0.0f, knockbackForce)),
      radius(std::max(0.01f, radius)),
      lifetime(std::max(0.01f, lifetime)),
      gravity(std::max(0.0f, gravity)),
      groundY(groundY),
      sourceFaction(sourceFaction),
      parryable(parryable),
      hasGround(hasGround) {}

bool Projectile::canHit(const Character& target) const {
  if (!target.getIsActive()) {
    return false;
  }

  if (sourceFaction == Faction::Neutral || target.getFaction() == Faction::Neutral) {
    return false;
  }

  return sourceFaction != target.getFaction();
}

void Projectile::update(float dt, const std::vector<Character*>& targets) {
  if (!active || dt <= 0.0f) {
    return;
  }

  lifetime -= dt;
  if (lifetime <= 0.0f) {
    lifetime = 0.0f;
    active = false;
    return;
  }

  const QVector2D previousPosition = position;
  if (gravity > 0.0f) {
    velocity.setY(velocity.y() + gravity * dt);
  }

  position += velocity * dt;

  const bool hitGround = hasGround && position.y() >= groundY;
  if (hitGround) {
    position.setY(groundY);
  }

  const float hitRadiusSq = radius * radius;
  for (Character* target : targets) {
    if (target == nullptr || !canHit(*target)) {
      continue;
    }

    if (const auto* player = dynamic_cast<const Player*>(target);
        player != nullptr && player->isRolling()) {
      continue;
    }

    bool intersectsTarget = false;
    if (dynamic_cast<const Player*>(target) != nullptr) {
      const QVector2D feet = target->getPosition();
      QRectF body(feet.x() - 32.0f, feet.y() - 112.0f, 64.0f, 112.0f);
      body.adjust(-radius, -radius, radius, radius);
      intersectsTarget = segmentIntersectsRect(previousPosition, position, body);
    } else {
      const QVector2D segment = position - previousPosition;
      const float segmentLengthSq = segment.lengthSquared();
      float t = 0.0f;
      if (segmentLengthSq > 0.0001f) {
        t = std::clamp(QVector2D::dotProduct(target->getPosition() - previousPosition,
                                             segment) / segmentLengthSq,
                       0.0f, 1.0f);
      }
      const QVector2D closestPoint = previousPosition + segment * t;
      intersectsTarget = (target->getPosition() - closestPoint).lengthSquared() <= hitRadiusSq;
    }

    if (intersectsTarget) {
      const bool hit = target->takeDamage(damage, nullptr, parryable);

      if (hit && knockbackForce > 0.0f && !velocity.isNull()) {
        target->applyKnockback(velocity, knockbackForce);
      }

      active = false;
      return;
    }
  }

  if (hitGround) {
    active = false;
  }
}
