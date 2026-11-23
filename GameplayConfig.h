#pragma once

namespace GameplayConfig {
inline constexpr int playerHealth = 100;
inline constexpr int silbonHealth = 180;
inline constexpr int turecoHealth = 70;

inline constexpr float silbonMeleeCooldown = 1.15f;
inline constexpr float boneCooldown = 6.4f;
inline constexpr float turecoKickCooldown = 8.0f;
inline constexpr float turecoAttachedWait = 1.8f;
inline constexpr float boneInitialDelay = 1.0f;
inline constexpr float kickInitialDelay = 4.0f;
inline constexpr float specialAttackSpacing = 2.5f;
inline constexpr float playerMeleeCooldown = 0.42f;
inline constexpr float turecoMeleeCooldown = 0.60f;

inline constexpr float turecoKickSpeed = 560.0f;
inline constexpr float turecoReturnAcceleration = 720.0f;
inline constexpr float movementFriction = 8.0f;
inline constexpr float playerRollCooldown = 0.85f;

inline constexpr int playerMeleeDamage = 24;
inline constexpr int silbonMeleeDamage = 12;
inline constexpr int boneDamage = 18;
inline constexpr int turecoDamage = 16;

inline constexpr float playerRollDuration = 0.32f;
inline constexpr float playerRollDistance = 180.0f;
inline constexpr float playerParryWindow = 0.28f;
inline constexpr float playerJumpSpeed = 620.0f;
inline constexpr float lateralGravity = 1500.0f;
inline constexpr float levelGroundY = 570.0f;
inline constexpr float playerMoveSpeed = 280.0f;
inline constexpr float silbonMoveSpeed = 105.0f;
inline constexpr float meleeRange = 100.0f;
inline constexpr float turecoAttachedOffset = 38.0f;
inline constexpr float parryRecovery = 0.30f;
inline constexpr float parryCooldown = 0.60f;
inline constexpr float boneProjectileSpeed = 400.0f;
inline constexpr float boneProjectileGravity = 900.0f;
inline constexpr float boneProjectileRadius = 18.0f;
inline constexpr float boneProjectileLifetime = 2.2f;
inline constexpr float meleeKnockback = 230.0f;
inline constexpr float boneLaunchHeight = 68.0f;
inline constexpr float boneMinimumFlightTime = 0.45f;
inline constexpr float boneMaximumFlightTime = 1.50f;
inline constexpr float playerMeleeRange = 112.0f;
inline constexpr float playerMeleeHalfWidth = 52.0f;
inline constexpr float enemyMeleeHalfWidth = 48.0f;
inline constexpr float turecoMeleeRange = 90.0f;
inline constexpr float turecoMeleeHalfWidth = 42.0f;
inline constexpr float playerWeight = 1.0f;
inline constexpr float enemyWeight = 1.5f;
inline constexpr float silbonWeight = 10000.0f;
inline constexpr float silbonBodyHeight = 184.0f;
inline constexpr float gameTickSeconds = 1.0f / 60.0f;
}
