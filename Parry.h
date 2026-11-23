#pragma once

class Character;

class Parry {
  private:
    float activeWindow;
    float recoveryDuration;
    float cooldown;
    float stunDuration;
    float deflectKnockback;

    float activeTimer   = 0.0f;
    float recoveryTimer = 0.0f;
    float cooldownTimer = 0.0f;

  public:
    Parry(float activeWindow,
          float recoveryDuration,
          float cooldown,
          float stunDuration,
          float deflectKnockback = 250.0f);

    bool execute();
    void update(float dt);
    bool tryDeflect(Character& owner, Character* attacker);

    bool isActive() const { return activeTimer > 0.0f; }
    bool isRecovering() const { return activeTimer <= 0.0f && recoveryTimer > 0.0f; }
    bool isBusy() const { return activeTimer > 0.0f || recoveryTimer > 0.0f; }
    bool isReady() const { return cooldownTimer <= 0.0f && !isBusy(); }
};
