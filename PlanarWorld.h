#pragma once

#include <QRectF>
#include <QVector2D>
#include <memory>
#include <vector>

#include "Agent.h"
#include "Doppel.h"
#include "Errant.h"
#include "PlanarPlayer.h"
#include "WaveSpawner.h"
#include "agentTypes.h"

enum class PlanarWorldState {
  Playing,
  ReadyForBossFight,
  GameOver
};

struct PlanarInput {
  QVector2D moveDirection;
  QVector2D aimDirection;
  bool attackPressed = false;
  bool rollPressed = false;
  bool parryPressed = false;
};

class PlanarWorld {
  private:
    AgentKnowledge sharedAiKnowledge;

    std::unique_ptr<PlanarPlayer> player;
    std::vector<std::unique_ptr<Errant>> errants;
    std::vector<std::unique_ptr<Doppel>> doppels;
    WaveSpawner spawner;

    PlanarWorldState state = PlanarWorldState::Playing;

    float elapsedTime = 0.0f;
    float drunkTimer = 0.0f;

    int doppelsDefeated = 0;
    bool bossPortalUnlocked = false;

    playerAction latchedPlayerAction = playerAction::INACTIVE;
    float actionLatchTimer = 0.0f;

    std::vector<Character*> buildWorldTargets() const;
    playerAction processPlayerInput(float dt,
                                    const PlanarInput& input,
                                    const std::vector<Character*>& targets);
    QVector2D getReferenceDirectionForPlayer() const;
    playerAction classifyDirectionAction(QVector2D dir,
                                         playerAction forward,
                                         playerAction away,
                                         playerAction right,
                                         playerAction left) const;

    void handleDeathsAndProgression();
    void clampToArena(Character& character) const;
    void checkBossPortal();

  public:
    PlanarWorld();

    void reset();
    void update(float dt, const PlanarInput& input);
    void saveAllKnowledge() const;

    // METODOS DE LECTURA PARA Level1GameWidget

    PlanarWorldState getState() const { return state; }
    bool isPlayerDrunk() const { return drunkTimer > 0.0f; }
    float getDrunkRemainingTime() const { return drunkTimer; }
    float getElapsedTime() const { return elapsedTime; }

    int getDoppelsSpawned() const { return spawner.getDoppelsSpawned(); }
    int getDoppelsDefeated() const { return doppelsDefeated; }
    bool isBossPortalUnlocked() const { return bossPortalUnlocked; }
    QRectF getBossPortalRect() const;

    const PlanarPlayer& getPlayer() const { return *player; }
    PlanarPlayer& getPlayer() { return *player; }

    const std::vector<std::unique_ptr<Errant>>& getErrants() const { return errants; }
    const std::vector<std::unique_ptr<Doppel>>& getDoppels() const { return doppels; }
};
