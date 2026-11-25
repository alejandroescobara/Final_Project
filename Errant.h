#pragma once

#include <QVector2D>
#include <vector>

#include "Character.h"

class Player;

class Errant final : public Character {
  private:
    int contactDamage;
    int xpReward;
    bool hitPlayer = false;

  public:
    using Character::update;

    explicit Errant(QVector2D initialPosition, int level = 1);

    void update(float dt,
                Player& player,
                const std::vector<Character*>& targets);

    // METODOS TRIVIALES

    int getContactDamage() const { return contactDamage; }
    int getXpReward() const { return xpReward; }
    bool didHitPlayer() const { return hitPlayer; }
};
