#pragma once

#include <QVector2D>
#include <vector>

class Character;

class StatusEffect {
  protected:
    float remainingDuration;

  public:
    explicit StatusEffect(float duration);
    virtual ~StatusEffect() = default;

    void update(float dt);
    bool isFinished() const { return remainingDuration <= 0.0f; }

    virtual void onTick(Character& target, float dt, const std::vector<Character*>& worldCharacters = {}) {}
    virtual bool canAct() const { return true; }
    virtual void modifyMoveDirection(QVector2D& dir) {}
    virtual void modifyIncomingDamage(int& damage) {}
};

// <| STUNT |>

class StunEffect : public StatusEffect {
  public:
    explicit StunEffect(float duration) : StatusEffect(duration) {}
    bool canAct() const override { return false; }
};

// <| INVULNERABILITY |>

class InvulnerabilityEffect : public StatusEffect {
  public:
    explicit InvulnerabilityEffect(float duration) : StatusEffect(duration) {}
    void modifyIncomingDamage(int& damage) override { damage = 0; }
};

// <| DRUNK |>

class DrunkEffect : public StatusEffect {
  private:
    QVector2D currentDir{0.0f, 0.0f};
    float lastDt = 0.016f;
    float responsiveness; // Menor valor = más inercia y giro más lento

  public:
    DrunkEffect(float duration, float turnResponsiveness = 4.0f);
    void onTick(Character& target, float dt, const std::vector<Character*>& worldCharacters = {}) override;
    void modifyMoveDirection(QVector2D& dir) override;
};
