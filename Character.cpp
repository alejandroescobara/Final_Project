#include "Character.h"

#include <QtMath>
#include <algorithm>

#include "Attack.h"
#include "StatusEffect.h"
#include "PhysicsMotor.h"

namespace {
  Direction vectorToDirection(QVector2D dir) {
    constexpr float SECTOR = static_cast<float>(2.0 * M_PI) / DIRECTION_COUNT;

    const float angle = std::atan2(dir.y(), dir.x());
    const int sectorOffset = static_cast<int>(std::round(angle / SECTOR));
    const int index = (sectorOffset % DIRECTION_COUNT + DIRECTION_COUNT) % DIRECTION_COUNT;

    return static_cast<Direction>(index);
  }
}

void Character::updatePhysics(float dt) {
  if (physics) {
    physics->update(position, moveVelocity, externalVelocity, dt);
  }
}

void Character::updateFacingDirection(QVector2D dir) {
  if (!dir.isNull()) {
    facingDirection = vectorToDirection(dir);
  }
}

Character::Character(QVector2D initialPos, int maxHp, float speed, float mass,
                     std::unique_ptr<PhysicsMotor> physicsMotor,
                     Faction initialFaction)
          : hp(std::max(1, maxHp)),
            maxHp(std::max(1, maxHp)),
            weight(std::max(0.01f, mass)),
            faction(initialFaction),
            position(initialPos),
            baseMoveSpeed(std::max(0.0f, speed)),
            physics(std::move(physicsMotor)) {}

Character::~Character() = default;

void Character::move(QVector2D dir) {
  if (!isActive || !canAct()) {
    return;
  }

  for (const auto& effect : activeStatusEffects) {
    effect->modifyMoveDirection(dir);
  }

  if (physics) {
    dir = physics->filterDirection(dir);
  }

  if (dir.isNull()) {
    moveVelocity = QVector2D(0.0f, 0.0f);
    return;
  }

  if (dir.lengthSquared() > 1.0f) {
    dir.normalize();
  }

  moveVelocity = dir * baseMoveSpeed;
  updateFacingDirection(dir);
}

void Character::applyKnockback(QVector2D dir, float force) {
  if (!isActive || dir.isNull() || force <= 0.0f) {
    return;
  }

  const float effectiveForce = force / weight;
  externalVelocity += dir.normalized() * effectiveForce;
}

bool Character::attack(QVector2D dir, const std::vector<Character*>& targets) {
  if (!isActive || !canAct() || !primaryAttack || dir.isNull()) {
    return false;
  }

  if (primaryAttack->execute(*this, dir.normalized(), targets)) {
    updateFacingDirection(dir);
    return true;
  }

  return false;
}

bool Character::takeDamage(int amount, Character* /*attacker*/, bool /*parryable*/){
  if (!isActive || amount <= 0) return false;

  for(const auto& effect : activeStatusEffects) {
    effect->modifyIncomingDamage(amount);
  }

  if (amount <= 0) return false;

  hp = std::max(0, hp - amount);

  if (hp == 0) {
    isActive = false;
    moveVelocity = QVector2D(0.0f, 0.0f);
    externalVelocity = QVector2D(0.0f, 0.0f);
    activeStatusEffects.clear();
  }

  return true;
}

void Character::heal(int amount) {
  if (!isActive || amount <= 0) return;

  hp = std::min(maxHp, hp + amount);
}

void Character::update(float dt, const std::vector<Character*>& worldCharacters) {
  if (!isActive || dt <= 0.0f) return;

  for (auto& effect : activeStatusEffects) {
    effect->update(dt);
    effect->onTick(*this, dt, worldCharacters);
  }

  std::erase_if(activeStatusEffects, [](const auto& effect) {
    return effect->isFinished();
  });

  if (primaryAttack) {
    primaryAttack->update(dt);
  }

  updatePhysics(dt);
}

void Character::addStatusEffect(std::unique_ptr<StatusEffect> effect) {
  if (!isActive || !effect) return;

  if (!effect->canAct()) {
    moveVelocity = QVector2D(0.0f, 0.0f);
  }

  activeStatusEffects.push_back(std::move(effect));
}

bool Character::canAct() const {
  for (const auto& effect: activeStatusEffects) {
    if (!effect->canAct()) return false;
  }
  return true;
}

void Character::setMaxHp(int newMaxHp) {
  maxHp = std::max(1, newMaxHp);
  hp = std::min(hp, maxHp);
}
    
void Character::setWeight(float newWeight) {
  weight = std::max(0.01f, newWeight);
}

void Character::setBaseMoveSpeed(float newSpeed) {
  baseMoveSpeed = std::max(0.0f, newSpeed);
}

void Character::setPrimaryAttack(std::unique_ptr<Attack> newAttack) {
  primaryAttack = std::move(newAttack);
}

void Character::setPhysicsMotor(std::unique_ptr<PhysicsMotor> newPhysics) {
  physics = std::move(newPhysics);
}

bool Character::isHostileTo(const Character& other) const {
  if (&other == this) return false;

  if (faction == Faction::Neutral || other.faction == Faction::Neutral) {
    return false;
  }

  return faction != other.faction;
}

QVector2D Character::getFacingVector() const {
  
  constexpr float DIAG = 0.70710678f;
  
  switch (facingDirection) {
    case Direction::Right:     return QVector2D(1.0f, 0.0f);
    case Direction::DownRight: return QVector2D(DIAG, DIAG);
    case Direction::Down:      return QVector2D(0.0f, 1.0f);
    case Direction::DownLeft:  return QVector2D(-DIAG, DIAG);
    case Direction::Left:      return QVector2D(-1.0f, 0.0f);
    case Direction::UpLeft:    return QVector2D(-DIAG, -DIAG);
    case Direction::Up:        return QVector2D(0.0f, -1.0f);
    case Direction::UpRight:   return QVector2D(DIAG, -DIAG);
  }
  return QVector2D(0.0f, 1.0f);
}
