#include "Agent.h"

static double normalizeAngle(double angle) {
    while (angle > 180.0)  angle -= 360.0;
    while (angle < -180.0) angle += 360.0;
    return angle;
}

Agent::Agent(AgentKnowledge& sharedKnowledge)
    : knowledge(sharedKnowledge),
      lastState{},
      lastAction(doppelAction::PARRY),
      hasPreviousState(false),
      lastPlayerContext{},
      hasPreviousContext(false) {}

playerContext Agent::buildPlayerContext(const egocentricPerception& perception) const {

  playerContext context;
  context.threatDistance = distanceSector::FAR;
  context.threatAngle    = angleSector::FRONT;
  context.isSurrounded   = false;
  context.lastAction     = playerAction::INACTIVE;

  bool playerIsVisible = false;
  double minDistanceToPlayer = 999999.0;
  size_t nearbyThreatsCount = 0;

  for (const auto& entity : perception.visibleEntityArray) {
    if (entity.type == entityType::PLAYER) {
      playerIsVisible = true;
      context.lastAction = perception.lastPlayerAction;

      minDistanceToPlayer    = entity.distance;
      context.threatDistance = entity.getPlayerDistanceSector(); // Posición de este Doppel según Otoniel
      context.threatAngle    = entity.getPlayerAngleSector();    // Ángulo de este Doppel según Otoniel

      if (entity.getPlayerDistanceSector() != distanceSector::FAR) {
          nearbyThreatsCount++;
      }
      break;
    }
  }

  if (!playerIsVisible) {
      return context;
  }

  for (const auto& entity : perception.visibleEntityArray) {
    if (entity.type != entityType::PLAYER) {

      if (entity.getPlayerDistanceSector() != distanceSector::FAR) {
        nearbyThreatsCount++;
      }

      if (entity.distanceToPlayer < minDistanceToPlayer) {
        minDistanceToPlayer    = entity.distanceToPlayer;
        context.threatDistance = entity.getPlayerDistanceSector();
        context.threatAngle    = entity.getPlayerAngleSector();
      }
    }
  }

  context.isSurrounded = (nearbyThreatsCount > 1);
  return context;
}

egocentricState Agent::buildEgocentricState(const egocentricPerception& perception, 
                                           playerAction predictedAction) const {
  egocentricState state;
  state.playerDistance        = distanceSector::FAR;
  state.playerAngle           = angleSector::FRONT;
  state.allyNearby            = false;
  state.predictedPlayerAction = predictedAction;

  for (const auto& entity : perception.visibleEntityArray) {
    if (entity.type == entityType::PLAYER) {
      state.playerDistance = entity.getDistanceSector();
      state.playerAngle    = entity.getAngleSector();
    } 
    else if (entity.type == entityType::DOPPELGANGER) {
      if (entity.getDistanceSector() != distanceSector::FAR) {
        state.allyNearby = true;
      }
    }
  }

  return state;
}

doppelAngledAction Agent::calculateAngledAction(doppelAction action, 
                                               const egocentricPerception& perception) const {

  double targetAngle = 0.0;
  for (const auto& entity : perception.visibleEntityArray) {
    if (entity.type == entityType::PLAYER) {
      targetAngle = entity.angle;
      break;
    }
  }

  double finalAngle = targetAngle;

  switch (action) {
    case doppelAction::MOVE_FORWARD:
    case doppelAction::ROLL_FORWARD:
    case doppelAction::ATTACK:
    case doppelAction::PARRY:
      finalAngle = targetAngle;
      break;

    case doppelAction::MOVE_AWAY:
    case doppelAction::ROLL_AWAY:
      finalAngle = targetAngle + 180.0;
      break;

    case doppelAction::MOVE_RIGHT:
    case doppelAction::ROLL_RIGHT:
      finalAngle = targetAngle + 90.0;
      break;

    case doppelAction::MOVE_LEFT:
    case doppelAction::ROLL_LEFT:
      finalAngle = targetAngle - 90.0;
      break;
  }

  doppelAngledAction result;
  result.type  = action;
  result.angle = normalizeAngle(finalAngle);
  return result;
}


doppelAngledAction Agent::update(const egocentricPerception& perception) {
  if (hasPreviousContext && perception.lastPlayerAction != playerAction::INACTIVE) {
    knowledge.markov.registerTransition(lastPlayerContext, perception.lastPlayerAction);
  }

  playerContext context = buildPlayerContext(perception);
  lastPlayerContext  = context;
  hasPreviousContext = true;

  playerAction predictedAction = knowledge.markov.predictNextAction(context);
  egocentricState currentState = buildEgocentricState(perception, predictedAction);

  knowledge.qAgent.stepDecay();

  doppelAction chosenAction = knowledge.qAgent.selectAction(currentState);

  lastState        = currentState;
  lastAction       = chosenAction;
  hasPreviousState = true;

  return calculateAngledAction(chosenAction, perception);
}

void Agent::resetTracking() {
  hasPreviousState   = false;
  hasPreviousContext = false;
}

void Agent::applyReward(double reward, const egocentricPerception& newPerception) {
  if (!hasPreviousState) return;

  playerContext newContext     = buildPlayerContext(newPerception);
  playerAction predictedAction = knowledge.markov.predictNextAction(newContext);
  egocentricState newState     = buildEgocentricState(newPerception, predictedAction);

  knowledge.qAgent.updateQValue(lastState, lastAction, reward, newState);

  lastState = newState;
}

bool Agent::saveKnowledge(std::string_view markovPath, std::string_view qTablePath) const {
  return knowledge.save(markovPath, qTablePath);
}

bool Agent::loadKnowledge(std::string_view markovPath, std::string_view qTablePath) {
  return knowledge.load(markovPath, qTablePath);
}
