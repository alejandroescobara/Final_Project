#pragma once

#include <memory>

#include "Character.h"
#include "GameplayConfig.h"
#include "ProjectileAttack.h"

class BoneAttack;
class Tureco;

class Silbon final : public Character {
  private:
    std::unique_ptr<BoneAttack> boneAttack;
    float boneTimer = GameplayConfig::boneInitialDelay;
    float kickTimer = GameplayConfig::kickInitialDelay;
    float attachedWaitTimer = 0.0f;
    float specialAttackSpacingTimer = 0.0f;
    bool turecoWasKicked = false;

  public:
    Silbon(QVector2D initialPosition, ProjectileAttack::ProjectileSpawner spawner);
    ~Silbon() override;

    void update(float dt,
                const std::vector<Character*>& targets,
                QVector2D playerPosition,
                Tureco& tureco);
};
