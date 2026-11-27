#include "Item.h"

#include <algorithm>
#include <utility>

#include "Player.h"

Item::Item(QString name, QString description)
    : name(std::move(name)),
      description(std::move(description)) {}

// <| ROSARY |>

Rosary::Rosary(float regenBonus, float duration)
      : Item("Rosario", "Otorga regeneracion pasiva de vida"),
        regenBonus(std::max(0.0f, regenBonus)),
        duration(std::max(0.0f, duration)),
        remainingDuration(this->duration) {}


void Rosary::onEquip(Player& player) {
  player.setRegenRate(player.getRegenRate() + regenBonus);
}

void Rosary::onRemove(Player& player) {
  player.setRegenRate(player.getRegenRate() - regenBonus);
}

void Rosary::update(Player& /*player*/, float dt) {
  if (duration > 0.0f) {
    remainingDuration = std::max(0.0f, remainingDuration - dt);
  }
}

bool Rosary::isExpired() const {
  return duration > 0.0f && remainingDuration <= 0.0f;
}

float Rosary::getRemainingDuration() const {
  return duration > 0.0f ? remainingDuration : -1.0f;
}

void Rosary::refreshDuration(float newDuration) {
  duration = std::max(0.0f, newDuration);
  remainingDuration = duration;
}
