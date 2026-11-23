#pragma once

#include <QVector2D>
#include <functional>
#include <utility>
#include <vector>

#include "Attack.h"
#include "Projectile.h"

class ProjectileAttack : public Attack {
  public:
    using ProjectileSpawner = std::function<void(Projectile)>;

  private:
    float projectileSpeed;
    float projectileRadius;
    float projectileLifetime;
    float gravity;
    bool hasGround;
    float groundY;
    ProjectileSpawner spawner;

  protected:
    void onExecute(Character& attacker,
                   QVector2D dir,
                   const std::vector<Character*>& targets) override;

    virtual Projectile createProjectile(const Character& attacker,
                                        QVector2D dir) const;

    void spawnProjectile(Projectile projectile) const;

  public:
    ProjectileAttack(int damage,
                     float cooldown,
                     float projectileSpeed,
                     float projectileRadius,
                     float projectileLifetime,
                     ProjectileSpawner spawner = nullptr,
                     float knockbackForce = 0.0f,
                     float gravity = 0.0f,
                     bool parryable = true,
                     bool hasGround = false,
                     float groundY = 0.0f);

    void setProjectileSpeed(float newSpeed);
    void setProjectileRadius(float newRadius);
    void setProjectileLifetime(float newLifetime);
    void setGravity(float newGravity);

    // METODOS TRIVIALES

    void setGround(float newGroundY) {
      hasGround = true;
      groundY = newGroundY;
    }
    void clearGround() { hasGround = false; }
    void setSpawner(ProjectileSpawner newSpawner) { spawner = std::move(newSpawner); }

    float getProjectileSpeed() const { return projectileSpeed; }
    float getProjectileRadius() const { return projectileRadius; }
    float getProjectileLifetime() const { return projectileLifetime; }
    float getGravity() const { return gravity; }
    bool usesGround() const { return hasGround; }
    float getGroundY() const { return groundY; }
};
