#include "Player.h"

#include <algorithm>
#include <ranges>

#include "Item.h"
#include "PhysicsMotor.h"

Player::Player(QVector2D initialPos, int maxHealth, float baseMoveSpeed, float weight,
               std::unique_ptr<PhysicsMotor> physicsMotor,
               const Roll& rollConfig, const Parry& parryConfig)
      : Character(initialPos, maxHealth, baseMoveSpeed, weight, std::move(physicsMotor), Faction::Player),
        roll(rollConfig),
        parry(parryConfig) {}

Player::~Player() = default;

void Player::update(float dt, const std::vector<Character*>& worldCharacters) {
  if (!isActive || dt <= 0.0f) return;

  roll.update(*this, dt);
  parry.update(dt);

  Character::update(dt, worldCharacters);

  for (auto item = equippedItems.begin(); item != equippedItems.end();) {
    (*item)->update(*this, dt);
    if ((*item)->isExpired()) {
      (*item)->onRemove(*this);
      item = equippedItems.erase(item);
    } else {
      ++item;
    }
  }

  updateRegen(dt);
}

void Player::updateRegen(float dt) {
  if (regenRate <= 0.0f || getHp() >= getMaxHp()) {
    return;
  }

  regenAccumulator += regenRate * dt;

  if (regenAccumulator >= 1.0f) {
    const int healAmount = static_cast<int>(regenAccumulator);
    regenAccumulator -= static_cast<float>(healAmount);
    heal(healAmount);
  }
}

bool Player::takeDamage(int amount, Character* attacker, bool parryable) {
  if (!isActive || amount <= 0) return false;

  if (parryable && parry.tryDeflect(*this, attacker)) {
    return false;
  }

  return Character::takeDamage(amount, attacker, parryable);
}

bool Player::canAct() const {
  return !roll.isActive() && !parry.isBusy() && Character::canAct();
}

bool Player::executeParry() {
  if (!isActive || !canAct()) return false;

  if (parry.execute()) {
    moveVelocity = QVector2D(0.0f, 0.0f);
    return true;
  }

  return false;
}

bool Player::executeRoll(QVector2D dir) {
  if (!isActive || !canAct()) {
    return false;
  }

  if (dir.isNull()) {
    dir = getFacingVector();
  }

  if (physics) {
    dir = physics->filterDirection(dir);
  }

  if (roll.execute(*this, dir)) {
    if (!dir.isNull()) {
      updateFacingDirection(dir);
    }
    return true;
  }
  return false;
}

void Player::equipItem(std::unique_ptr<Item> item) {
  if (!item) return;

  item->onEquip(*this);
  equippedItems.push_back(std::move(item));
}

void Player::removeItem(Item* item) {
  if (!item) return;

  auto it = std::ranges::find(equippedItems, item, &std::unique_ptr<Item>::get);
  if (it == equippedItems.end()) {
    return;
  }

  (*it)->onRemove(*this);
  equippedItems.erase(it);
}

void Player::removeItem(const QString& itemName) {
  
  auto it = std::ranges::find(equippedItems, itemName, &Item::getName);
  
  if (it == equippedItems.end()) {
    return;
  }

  (*it)->onRemove(*this);
  equippedItems.erase(it);
}

void Player::setRegenRate(float newRegenRate) {
  regenRate = std::max(0.0f, newRegenRate);
  if (regenRate == 0.0f) {
    regenAccumulator = 0.0f;
  }
}
