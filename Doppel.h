#pragma once

#include <QVector2D>
#include <vector>

#include "Agent.h"
#include "Character.h"
#include "Parry.h"
#include "Roll.h"

class Player;

class Doppel final : public Character {
  private:
    Agent brains;
    Roll roll;
    Parry parry;

    int xpReward;

    doppelAngledAction currentAction{doppelAction::MOVE_FORWARD, 0.0};
    QVector2D currentActionDir;
    egocentricPerception lastPerception;

    float decisionTimer = 0.0f;
    double pendingReward = 0.0;

    bool wasSeeingPlayer = false;
    bool hasActiveAction = false;

    egocentricPerception buildPerception(const Player& player,
                                         const std::vector<Character*>& targets,
                                         playerAction lastPlayerAction) const;
    void executeCurrentAction(Player& player, const std::vector<Character*>& targets);
    bool executeRoll(QVector2D dir);
    bool executeParry();

  public:
    Doppel(QVector2D initialPosition, AgentKnowledge& sharedKnowledge, int level = 1);

    void update(float dt, const std::vector<Character*>& worldCharacters = {}) override;
    void update(float dt,
                Player& player,
                const std::vector<Character*>& targets,
                playerAction lastPlayerAction = playerAction::INACTIVE);

    bool takeDamage(int amount,
                    Character* attacker = nullptr,
                    bool parryable = true) override;
    bool canAct() const override;

    // METODOS TRIVIALES

    Agent& getBrains() { return brains; }
    const Agent& getBrains() const { return brains; }

    int getXpReward() const { return xpReward; }
    bool isRolling() const { return roll.isActive(); }
    bool isParrying() const { return parry.isActive(); }
    bool isParryBusy() const { return parry.isBusy(); }
    bool isAiActive() const { return wasSeeingPlayer; }
};
