#include "Item.h"

#include <algorithm>
#include <utility>

#include "Player.h"

Item::Item(QString name, QString description)
    : name(std::move(name)),
      description(std::move(description)) {}

// <| ROSARY |>

Rosary::Rosary(float regenBonus)
      : Item("Rosario", "Otorga regeneracion pasiva de vida"),
        regenBonus(std::max(0.0f, regenBonus)) {}


void Rosary::onEquip(Player& player) {
  player.setRegenRate(player.getRegenRate() + regenBonus);
}

void Rosary::onRemove(Player& player) {
  player.setRegenRate(player.getRegenRate() - regenBonus);
}
