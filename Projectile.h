#pragma once

#include <QVector2D>
#include <vector>

#include "Character.h"

class Projectile {
  private:
    QVector2D position;
    QVector2D velocity;

    int damage;
    float knockbackForce;
    float radius;
    float lifetime;
    float gravity;
    float groundY;

    Faction sourceFaction;

    bool parryable;
    bool hasGround;
    bool active = true;

    bool canHit(const Character& target) const;

  public:
    Projectile(QVector2D initialPos,
               QVector2D velocity,
               int damage,
               float radius,
               float lifetime,
               Faction sourceFaction,
               float knockbackForce = 0.0f,
               float gravity = 0.0f,
               bool parryable = true,
               bool hasGround = false,
               float groundY = 0.0f);

    void update(float dt, const std::vector<Character*>& targets);

    // METODOS TRIVIALES

    bool isActive() const { return active; }
    
    void deactivate() { active = false; }

    QVector2D getPosition() const { return position; }
    QVector2D getVelocity() const { return velocity; }
    
    int getDamage() const { return damage; }
    
    float getKnockbackForce() const { return knockbackForce; }
    float getRadius() const { return radius; }
    float getLifetime() const { return lifetime; }
    float getGravity() const { return gravity; }
    float getGroundY() const { return groundY; }

    bool usesGround() const { return hasGround; }
    bool isParryable() const { return parryable; }

    Faction getSourceFaction() const { return sourceFaction; }
};
