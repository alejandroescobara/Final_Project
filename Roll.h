#pragma once

#include <QVector2D>

class Character;

class Roll {
  private:
    float speed;
    float duration;
    float cooldown;
    float invulnerabilityDuration;

    float activeTimer = 0.0f;
    float cooldownTimer = 0.0f;
    QVector2D rollDirection;

  public:
    Roll(float speed, float duration, float cooldown, float invulnerabilityDuration = -1.0f);

    bool execute(Character& owner, QVector2D dir);
    void update(Character& owner, float dt);

    bool isActive() const { return activeTimer > 0.0f; }
    bool isReady() const { return cooldownTimer <= 0.0f && !isActive(); }
};
