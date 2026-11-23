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

    const QString& getName() const { return name; }
    const QString& getDescription() const { return description; }
};

class Rosary: public Item {
  private:
    float regenBonus;

  public:
    explicit Rosary(float regenBonus = 1.0f);

    void onEquip(Player& player)  override;
    void onRemove(Player& player) override;
};
