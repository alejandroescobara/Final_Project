#pragma once

#include <QVector2D>
#include <vector>

class Character;

class Attack {
  private:
    int damage;
    
    float cooldown;
    float cooldownTimer = 0.0f;

    bool parryable;

  protected: 
    float force;

    Attack(int damage, float cooldown, float knockbackForce = 0.0f, bool parryable = true);
    
    virtual void onExecute(Character& Attacker, QVector2D dir, const std::vector<Character*>& targets) = 0;
    virtual bool isValidTarget(const Character& attacker, const Character& target) const;
  public:
    virtual ~Attack() = default;

    void update(float dt);
    bool execute(Character& attacker, QVector2D dir, const std::vector<Character*>& targets);

    void setDamage(int newDamage);
    void setCooldown(float newCooldown);

    // METODOS TRIVIALES

    void setForce(float newForce) { force = newForce; }
    void setParryable(bool newParryable) { parryable = newParryable; }

    bool isReady() const { return cooldownTimer <= 0.0f; }
    int getDamage() const { return damage; }
    float getCooldown() const { return cooldown; }
    float getCooldownTimer() const { return cooldownTimer; }
    float getForce() const { return force; }
    bool isParryable() const { return parryable; }
};
