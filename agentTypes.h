#pragma once

#include <vector>
#include <cmath>
#include <cstdint>

enum class distanceSector: uint8_t  { SHORT, MEDIUM, FAR};

enum class angleSector: uint8_t     { FRONT, SIDE, BACK };

enum class playerAction: uint8_t {
  
  INACTIVE,

  MOVE_FORWARD,
  MOVE_AWAY,
  MOVE_RIGHT,
  MOVE_LEFT,

  ROLL_FORWARD,
  ROLL_AWAY,
  ROLL_RIGHT,
  ROLL_LEFT,

  ATTACK_FORWARD,
  ATTACK_AWAY,
  ATTACK_RIGHT,
  ATTACK_LEFT,

  PARRY
};

enum class doppelAction: uint8_t {
  
  MOVE_FORWARD,
  MOVE_AWAY,
  MOVE_RIGHT,
  MOVE_LEFT,

  ROLL_FORWARD,
  ROLL_AWAY,
  ROLL_RIGHT,
  ROLL_LEFT,

  ATTACK, // NOTE: se calcula el angulo de ataque directo hacia el jugador en tiempo real

  PARRY
};

struct doppelAngledAction {
  doppelAction type;
  double angle; //NOTE: En radianes o grados (-180° a 180°)
};

enum class entityType {
  PLAYER,
  ERRANT,
  DOPPELGANGER
};

struct VisibleEntity {
  entityType type;

  double distance;
  double angle;
                
  double distanceToPlayer;
  double angleToPlayer;

  distanceSector getDistanceSector() const {
    if (distance <= 60.0)  return distanceSector::SHORT;
    if (distance <= 180.0) return distanceSector::MEDIUM;
    return distanceSector::FAR;
  }

  angleSector getAngleSector() const {
    double absAngle = std::abs(angle);
    if (absAngle <= 45.0)  return angleSector::FRONT;
    if (absAngle <= 135.0) return angleSector::SIDE;
    return angleSector::BACK;
  }

  distanceSector getPlayerDistanceSector() const {
    if (distanceToPlayer <= 60.0)  return distanceSector::SHORT;
    if (distanceToPlayer <= 180.0) return distanceSector::MEDIUM;
    return distanceSector::FAR;
  }

  angleSector getPlayerAngleSector() const {
    double absAngle = std::abs(angleToPlayer);
    if (absAngle <= 45.0)  return angleSector::FRONT;
    if (absAngle <= 135.0) return angleSector::SIDE;
    return angleSector::BACK;
  }
};

struct egocentricPerception {
  std::vector<VisibleEntity> visibleEntityArray;
  playerAction lastPlayerAction;
};

struct playerContext {
  distanceSector threatDistance;
  angleSector    threatAngle;
  bool           isSurrounded;
  playerAction   lastAction;

  bool operator<(const playerContext& other) const {
    if (threatDistance != other.threatDistance)
        return threatDistance < other.threatDistance;
    if (threatAngle != other.threatAngle)
        return threatAngle < other.threatAngle;
    if (isSurrounded != other.isSurrounded)
        return isSurrounded < other.isSurrounded;
    return lastAction < other.lastAction;
  }
};

struct egocentricState {
  distanceSector playerDistance;
  angleSector playerAngle;
  bool allyNearby;
  playerAction predictedPlayerAction;

  bool operator<(const egocentricState& other) const {
    if (playerDistance != other.playerDistance)
        return playerDistance < other.playerDistance;
    if (playerAngle != other.playerAngle)
        return playerAngle < other.playerAngle;
    if (allyNearby != other.allyNearby)
        return allyNearby < other.allyNearby;
    return predictedPlayerAction < other.predictedPlayerAction;
  }
};
