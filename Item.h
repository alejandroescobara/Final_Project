#pragma once

#include <QString>

class Player;

class Item {
  private:
    QString name;
    QString description;
  public:
    Item(QString name, QString description = "");
    virtual ~Item() = default;

    virtual void onEquip(Player& player) = 0;
    virtual void onRemove(Player& /*player*/) {}
    virtual void update(Player& /*player*/, float /*dt*/) {}
    virtual bool isExpired() const { return false; }
    virtual float getRemainingDuration() const { return -1.0f; }

    const QString& getName() const { return name; }
    const QString& getDescription() const { return description; }
};

class Rosary: public Item {
  private:
    float regenBonus;
    float duration;
    float remainingDuration;

  public:
    explicit Rosary(float regenBonus = 1.0f, float duration = 0.0f);

    void onEquip(Player& player) override;
    void onRemove(Player& player) override;
    void update(Player& player, float dt) override;
    bool isExpired() const override;
    float getRemainingDuration() const override;
    void refreshDuration(float newDuration);
};
