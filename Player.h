#pragma once

#include <memory>
#include <vector>
#include <QVector2D>

#include "Character.h"
#include "Parry.h"
#include "Roll.h"

class Item;
class PhysicsMotor;

class Player : public Character {
  private:
    std::vector<std::unique_ptr<Item>> equippedItems;

    float regenRate = 0.0f;
    float regenAccumulator = 0.0f;

    void updateRegen(float dt);

  protected:
    Roll roll;
    Parry parry;

    Player(QVector2D initialPos,
           int maxHealth,
           float baseMoveSpeed,
           float weight,
           std::unique_ptr<PhysicsMotor> physicsMotor,
           const Roll& rollConfig,
           const Parry& parryConfig);

  public:
    ~Player() override;

    void update(float dt, const std::vector<Character*>& worldCharacters = {}) override;
    bool takeDamage(int amount,
                    Character* attacker = nullptr,
                    bool parryable = true) override;
    bool canAct() const override;

    virtual bool executeParry();
    virtual bool executeRoll(QVector2D dir);

    void equipItem(std::unique_ptr<Item> item);
    void removeItem(Item* item);
    void removeItem(const QString& itemName);
    
    void setRegenRate(float newRegenRate);
    float getRegenRate() const { return regenRate; }

    bool isRolling() const { return roll.isActive(); }
    bool isParrying() const { return parry.isActive(); }
    bool isParryRecovering() const { return parry.isRecovering(); }
    bool isParryBusy() const { return parry.isBusy(); }

    const std::vector<std::unique_ptr<Item>>& getEquippedItems() const {
      return equippedItems;
    }
};
