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
    bool playerInMeleeRange = false;
    float meleeAttackTimer = 0.0f;

  public:
    Silbon(QVector2D initialPosition,
           ProjectileAttack::ProjectileSpawner spawner,
           int meleeDamage = GameplayConfig::silbonMeleeDamage,
           int boneDamage = GameplayConfig::boneDamage,
           int boneProjectileCount = 1);
    ~Silbon() override;

    void update(float dt,
                const std::vector<Character*>& targets,
                QVector2D playerPosition,
                Tureco& tureco);
};


