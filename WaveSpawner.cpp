#include "WaveSpawner.h"

#include <QtMath>
#include <algorithm>
#include <cmath>

#include "Doppel.h"
#include "Errant.h"
#include "GameplayConfig.h"

namespace {
  constexpr float INITIAL_ERRANT_DELAY = 0.5f;
  constexpr int MAX_SPAWN_ATTEMPTS = 20;
}  // namespace

WaveSpawner::WaveSpawner()
    : rng(std::random_device{}()) {
  reset();
}

void WaveSpawner::reset() {
  errantSpawnTimer = INITIAL_ERRANT_DELAY;
  doppelSpawnTimer = GameplayConfig::level1FirstDoppelDelay;
  doppelsSpawned = 0;
}

void WaveSpawner::update(float dt,
                         QVector2D playerPos,
                         bool isPlayerDrunk,
                         int doppelsDefeated,
                         std::vector<std::unique_ptr<Errant>>& errants,
                         std::vector<std::unique_ptr<Doppel>>& doppels) {
  if (dt <= 0.0f) return;

  const int enemyLevel = computeEnemyLevel(doppelsDefeated);

  // 1. Errants: salen desde el segundo 0, con tope reducido mientras dura el estado borracho
  errantSpawnTimer -= dt;
  if (errantSpawnTimer <= 0.0f) {
    const int maxErrants = isPlayerDrunk
                               ? GameplayConfig::level1MaxErrantsWhileDrunk
                               : GameplayConfig::level1MaxActiveErrants;
    if (static_cast<int>(errants.size()) < maxErrants) {
      spawnErrant(playerPos, enemyLevel, errants);
    }
    errantSpawnTimer = GameplayConfig::level1ErrantSpawnInterval;
  }

  // 2. Doppels:
  //    - #1 aparece durante el estado borracho (pelear o huir)
  //    - Tras pasarse el efecto: #2 solo, #3 solo, #4 y #5 juntos
  //    - Modo infinito: siguen apareciendo de a 2
  if (doppelsSpawned == 0) {
    doppelSpawnTimer -= dt;
    if (doppelSpawnTimer <= 0.0f) {
      spawnDoppel(playerPos, enemyLevel, doppels);
      doppelSpawnTimer = GameplayConfig::level1DoppelRespawnDelay;
    }
    return;
  }

  if (isPlayerDrunk || !doppels.empty()) {
    return;
  }

  doppelSpawnTimer -= dt;
  if (doppelSpawnTimer <= 0.0f) {
    const int pairThreshold = std::max(1, GameplayConfig::level1DoppelsForBoss - 2);
    if (doppelsSpawned < pairThreshold) {
      spawnDoppel(playerPos, enemyLevel, doppels);
    } else {
      spawnDoppel(playerPos, enemyLevel, doppels);
      spawnDoppel(playerPos, enemyLevel, doppels);
    }
    doppelSpawnTimer = GameplayConfig::level1DoppelRespawnDelay;
  }
}

void WaveSpawner::spawnErrant(QVector2D playerPos,
                              int enemyLevel,
                              std::vector<std::unique_ptr<Errant>>& errants) {
  errants.push_back(std::make_unique<Errant>(pickSpawnPosition(playerPos), enemyLevel));
}

void WaveSpawner::spawnDoppel(QVector2D playerPos,
                              int enemyLevel,
                              std::vector<std::unique_ptr<Doppel>>& doppels) {
  auto doppel = std::make_unique<Doppel>(pickSpawnPosition(playerPos), enemyLevel);
  doppel->getBrains().loadKnowledge();
  doppels.push_back(std::move(doppel));
  ++doppelsSpawned;
}

QVector2D WaveSpawner::pickSpawnPosition(QVector2D playerPos) {
  const float margin = GameplayConfig::level1BoundaryMargin;
  const float maxX = GameplayConfig::level1MapWidth - margin;
  const float maxY = GameplayConfig::level1MapHeight - margin;

  std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * static_cast<float>(M_PI));
  std::uniform_real_distribution<float> radiusDist(
      GameplayConfig::level1MinSpawnDistance,
      GameplayConfig::level1MaxSpawnDistance);

  for (int attempt = 0; attempt < MAX_SPAWN_ATTEMPTS; ++attempt) {
    const float angle = angleDist(rng);
    const float radius = radiusDist(rng);
    const QVector2D candidate(
        playerPos.x() + std::cos(angle) * radius,
        playerPos.y() + std::sin(angle) * radius);

    if (candidate.x() >= margin && candidate.x() <= maxX &&
        candidate.y() >= margin && candidate.y() <= maxY) {
      return candidate;
    }
  }

  // Respaldo: si el jugador está en una esquina, spawnea hacia el centro del mapa
  const QVector2D mapCenter(GameplayConfig::level1MapWidth * 0.5f,
                            GameplayConfig::level1MapHeight * 0.5f);
  QVector2D toCenter = mapCenter - playerPos;
  if (toCenter.isNull()) {
    toCenter = QVector2D(1.0f, 0.0f);
  } else {
    toCenter.normalize();
  }

  const QVector2D fallback = playerPos + toCenter * GameplayConfig::level1MinSpawnDistance;
  return QVector2D(std::clamp(fallback.x(), margin, maxX),
                   std::clamp(fallback.y(), margin, maxY));
}

int WaveSpawner::computeEnemyLevel(int doppelsDefeated) const {
  return 1 + (doppelsDefeated / 2);
}
