#pragma once

#include <QVector2D>
#include <memory>
#include <random>
#include <vector>

struct AgentKnowledge;
class Doppel;
class Errant;

class WaveSpawner {
  private:
    float errantSpawnTimer = 0.0f;
    float doppelSpawnTimer = 0.0f;
    int doppelsSpawned = 0;

    std::mt19937 rng;

    void spawnErrant(QVector2D playerPos,
                     int enemyLevel,
                     std::vector<std::unique_ptr<Errant>>& errants);
    void spawnDoppel(QVector2D playerPos,
                     int enemyLevel,
                     AgentKnowledge& sharedKnowledge,
                     std::vector<std::unique_ptr<Doppel>>& doppels);
    QVector2D pickSpawnPosition(QVector2D playerPos);
    int computeEnemyLevel(int doppelsDefeated) const;

  public:
    WaveSpawner();

    void reset();
    void update(float dt,
                QVector2D playerPos,
                bool isPlayerDrunk,
                int doppelsDefeated,
                AgentKnowledge& sharedKnowledge,
                std::vector<std::unique_ptr<Errant>>& errants,
                std::vector<std::unique_ptr<Doppel>>& doppels);

    // METODOS TRIVIALES

    int getDoppelsSpawned() const { return doppelsSpawned; }
};
