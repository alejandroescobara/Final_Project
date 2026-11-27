#pragma once

namespace GameplayConfig {

// ==================== Compartido y Nivel 2 (sin modificar para evitar conflictos) ====================
inline constexpr int playerHealth = 100;
inline constexpr int silbonHealth = 180;
inline constexpr int turecoHealth = 70;

inline constexpr float silbonMeleeCooldown = 1.15f;
inline constexpr float silbonMeleeInterval = 1.0f;
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
inline constexpr float level2PlatformOneX = 300.0f;
inline constexpr float level2PlatformOneY = 460.0f;
inline constexpr float level2PlatformOneWidth = 230.0f;
inline constexpr float level2PlatformTwoX = 690.0f;
inline constexpr float level2PlatformTwoY = 405.0f;
inline constexpr float level2PlatformTwoWidth = 230.0f;
inline constexpr float level2PlatformThickness = 14.0f;
inline constexpr float playerMoveSpeed = 280.0f;
inline constexpr float silbonMoveSpeed = 105.0f;
inline constexpr float meleeRange = 100.0f;
inline constexpr float turecoAttachedOffset = 38.0f;
inline constexpr float parryRecovery = 0.30f;
inline constexpr float parryCooldown = 0.60f;
inline constexpr float boneProjectileSpeed = 400.0f;
inline constexpr float boneProjectileGravity = 1200.0f;
inline constexpr float boneProjectileRadius = 18.0f;
inline constexpr float boneProjectileLifetime = 2.2f;
inline constexpr float meleeKnockback = 230.0f;
inline constexpr float boneLaunchHeight = 68.0f;
inline constexpr float boneMinimumFlightTime = 0.45f;
inline constexpr float boneMaximumFlightTime = 1.50f;
inline constexpr float playerMeleeRange = 112.0f;
inline constexpr float playerMeleeHalfWidth = 52.0f;
inline constexpr float level2PlayerMeleeHalfWidth = 180.0f;
inline constexpr float enemyMeleeHalfWidth = 48.0f;
inline constexpr float silbonMeleeHalfWidth = 170.0f;
inline constexpr float turecoMeleeRange = 90.0f;
inline constexpr float turecoMeleeHalfWidth = 42.0f;
inline constexpr float playerWeight = 1.0f;
inline constexpr float enemyWeight = 1.5f;
inline constexpr float silbonWeight = 10000.0f;
inline constexpr float silbonBodyHeight = 184.0f;
inline constexpr float gameTickSeconds = 1.0f / 60.0f;

// ==================== Nivel 1 (Vista en plano) ====================

inline constexpr float parryStunDuration = 0.8f;
inline constexpr float planarPlayerAttackAngle = 110.0f;
inline constexpr int planarBaseXpToLevel = 50;
inline constexpr int planarXpPerLevel = 25;
inline constexpr int level1RosaryDropChancePercent = 50;
inline constexpr float level1RosaryDuration = 10.0f;
inline constexpr float level1RosaryRegenPerSecond = 1.0f;

struct CharacterStats {
  int baseHp;
  int hpPerLevel;
  float baseSpeed;
  float speedPerLevel;
  float maxSpeed;
  float weight;
  int baseDamage;
  int damagePerLevel;
  float attackRange;
};

inline constexpr CharacterStats planarPlayer{
    .baseHp = playerHealth,
    .hpPerLevel = 20,
    .baseSpeed = playerMoveSpeed,
    .speedPerLevel = 2.5f,
    .maxSpeed = 310.0f,
    .weight = playerWeight,
    .baseDamage = playerMeleeDamage,
    .damagePerLevel = 6,
    .attackRange = playerMeleeRange,
};

inline constexpr int errantBaseXp = 15;
inline constexpr int errantXpPerLevel = 5;

inline constexpr CharacterStats errant{
    .baseHp = 24,
    .hpPerLevel = 12,
    .baseSpeed = 300.0f,
    .speedPerLevel = 5.0f,
    .maxSpeed = 350.0f,
    .weight = 0.8f,
    .baseDamage = 12,
    .damagePerLevel = 4,
    .attackRange = 36.0f,
};

inline constexpr int doppelBaseXp = 45;
inline constexpr int doppelXpPerLevel = 15;
inline constexpr float doppelAttackCooldown = 0.65f;
inline constexpr float doppelAttackAngle = 90.0f;
inline constexpr float doppelVisionRange = 360.0f;
inline constexpr float doppelDecisionInterval = 0.25f;
inline constexpr float doppelRollSpeed = playerRollDistance / playerRollDuration;
inline constexpr float doppelRollDuration = playerRollDuration;
inline constexpr float doppelRollCooldown = 1.15f;
inline constexpr float doppelParryWindow = 0.24f;
inline constexpr float doppelParryRecovery = 0.35f;
inline constexpr float doppelParryCooldown = 0.90f;

inline constexpr CharacterStats doppel{
    .baseHp = 90,
    .hpPerLevel = 25,
    .baseSpeed = 250.0f,
    .speedPerLevel = 6.0f,
    .maxSpeed = 315.0f,
    .weight = enemyWeight,
    .baseDamage = 18,
    .damagePerLevel = 5,
    .attackRange = 100.0f,
};

inline constexpr float level1CameraWidth = 960.0f;
inline constexpr float level1CameraHeight = 540.0f;

inline constexpr float level1MapWidth = 2800.0f;
inline constexpr float level1MapHeight = 2200.0f;
inline constexpr float level1BoundaryMargin = 32.0f;
inline constexpr float level1PortalTriggerDepth = 36.0f;

inline constexpr float level1MinSpawnDistance = 580.0f;
inline constexpr float level1MaxSpawnDistance = 820.0f;
inline constexpr float level1ErrantDespawnDistance = 1150.0f;

inline constexpr int level1DoppelsForBoss = 5;
inline constexpr int level1MaxErrantsWhileDrunk = 3;
inline constexpr int level1MaxActiveErrants = 5;
inline constexpr float level1ErrantSpawnInterval = 2.6f;
inline constexpr float level1FirstDoppelDelay = 3.0f;
inline constexpr float level1DoppelRespawnDelay = 2.5f;

inline constexpr float level1DrunkDuration = 30.0f;
inline constexpr float level1DrunkResponsiveness = 4.0f;
} 




