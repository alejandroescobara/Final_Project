#pragma once

#include <QVector2D>

#include "Player.h"

class PlanarPlayer final : public Player {
  private:
    int level = 1;
    int xp = 0;
    int xpToNextLevel;

    void levelUp();

  public:
    explicit PlanarPlayer(QVector2D initialPosition);

    bool addXp(int amount);

    // METODOS TRIVIALES

    int getLevel() const { return level; }
    int getXp() const { return xp; }
    int getXpToNextLevel() const { return xpToNextLevel; }
};
