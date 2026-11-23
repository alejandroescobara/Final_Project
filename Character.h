#pragma once

#include <memory>
#include <QVector2D>
#include <vector>

class Attack;
class StatusEffect;
class PhysicsMotor;

enum class Direction {
  Right,
  DownRight,
  Down,
  DownLeft,
  Left,
  UpLeft,
  Up,
  UpRight
};

constexpr int DIRECTION_COUNT = 8;

enum class Faction {
  Player,
  Enemy,
  Neutral
};

class Character {
  private:
    std::vector<std::unique_ptr<StatusEffect>> activeStatusEffects;

    int hp = 100;
    int maxHp = 100;

    float weight;
    Faction faction = Faction::Neutral;

  protected:
    QVector2D position;
    QVector2D moveVelocity;
    QVector2D externalVelocity;

    std::unique_ptr<Attack> primaryAttack;

    float baseMoveSpeed;

    bool isActive = true;

    Direction facingDirection = Direction::Down;

    std::unique_ptr<PhysicsMotor> physics;

    Character(QVector2D initialPos, int maxHp, float speed, float mass,
              std::unique_ptr<PhysicsMotor> physicsMotor,
              Faction initialFaction = Faction::Neutral);

    virtual void updatePhysics(float dt);

    virtual void updateFacingDirection(QVector2D dir);
    void updateFacingDirection(Direction newDirection) { facingDirection = newDirection; }

  public:
    virtual ~Character();

    virtual void move(QVector2D dir);
    void applyKnockback(QVector2D dir, float force);
    bool attack(QVector2D dir, const std::vector<Character*>& targets);
    
    virtual bool takeDamage(int damageAmount,
                            Character* attacker = nullptr,
                            bool parryable = true);
    void heal(int healAmount);
    virtual void update(float dt, const std::vector<Character*>& worldCharacters = {});

    void addStatusEffect(std::unique_ptr<StatusEffect> effect);
    virtual bool canAct() const;

    void setMaxHp(int newMaxHp);
    void setWeight(float newWeight);
    void setBaseMoveSpeed(float newSpeed);
    void setPhysicsMotor(std::unique_ptr<PhysicsMotor> newPhysics);
    PhysicsMotor* getPhysicsMotor() const { return physics.get(); }

    bool isHostileTo(const Character& other) const;

    // METODOS TRIVIALES

    int getHp() const { return hp; }

    int getMaxHp() const { return maxHp; }

    void setPosition(QVector2D newPos) { position = newPos; }
    QVector2D getPosition() const { return position; }
    
    void setMoveVelocity(QVector2D newVel) { moveVelocity = newVel; }
    QVector2D getMoveVelocity() const { return moveVelocity; }

    void setExternalVelocity(QVector2D newExternalVel) { externalVelocity = newExternalVel; }
    QVector2D getExternalVelocity() const { return externalVelocity; }

    void setPrimaryAttack(std::unique_ptr<Attack>newAttack);
    Attack* getPrimaryAttack() const { return primaryAttack.get(); }

    float getBaseMoveSpeed() const { return baseMoveSpeed; }

    float getWeight() const { return weight; }

    void setIsActive(bool newIsActive) { isActive = newIsActive; }
    bool getIsActive() const { return isActive; }

    Direction getFacingDirection() const { return facingDirection; }

    QVector2D getFacingVector() const;

    void setFaction(Faction newFaction) { faction = newFaction; }
    Faction getFaction() const { return faction; }
};
