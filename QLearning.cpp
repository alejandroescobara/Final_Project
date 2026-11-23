#include "QLearning.h"

#include <fstream>
#include <iostream>
#include <array>

static constexpr std::array<doppelAction, 10> ALL_DOPPEL_ACTIONS = {
  doppelAction::MOVE_FORWARD,
  doppelAction::MOVE_AWAY,
  doppelAction::MOVE_RIGHT,
  doppelAction::MOVE_LEFT,
  doppelAction::ROLL_FORWARD,
  doppelAction::ROLL_AWAY,
  doppelAction::ROLL_RIGHT,
  doppelAction::ROLL_LEFT,
  doppelAction::ATTACK,
  doppelAction::PARRY
};

QLearningAgent::QLearningAgent(double learningRate, double discountFactor, double explorationRate)
    : alpha(learningRate),
      gamma(discountFactor),
      epsilon(explorationRate),
      minEpsilon(0.02),
      decayFactor(0.997),
      gen(std::random_device{}()),
      dist(0.0, 1.0),
      actionDist(0, ALL_DOPPEL_ACTIONS.size() - 1) {}

doppelAction QLearningAgent::selectAction(const egocentricState& state) {
  if (dist(gen) < epsilon) {
    return ALL_DOPPEL_ACTIONS[actionDist(gen)];
  }

  auto stateIt = qTable.find(state);
  if (stateIt == qTable.end() || stateIt->second.empty()) {
    return ALL_DOPPEL_ACTIONS[actionDist(gen)];
  }

  doppelAction bestAction = ALL_DOPPEL_ACTIONS[0];
  double maxQ = -1e9;

  for (const auto& action : ALL_DOPPEL_ACTIONS) {
    double qVal = getQValue(state, action);
    if (qVal > maxQ) {
      maxQ = qVal;
      bestAction = action;
    }
  }

  return bestAction;
}

void QLearningAgent::updateQValue(const egocentricState& lastState,
                                  doppelAction actionTaken,
                                  double reward,
                                  const egocentricState& newState) {
//---------
  double currentQ = getQValue(lastState, actionTaken);

  double maxNextQ = -1e9;
  for (const auto& nextAction : ALL_DOPPEL_ACTIONS) {
    double nextQ = getQValue(newState, nextAction);
    if (nextQ > maxNextQ) {
      maxNextQ = nextQ;
    }
  }

  if (maxNextQ == -1e9) {
    maxNextQ = 0.0;
  }

  double newQ = currentQ + alpha * (reward + gamma * maxNextQ - currentQ);
  qTable[lastState][actionTaken] = newQ;
}

double QLearningAgent::getQValue(const egocentricState& state, doppelAction action) const {
  auto stateIt = qTable.find(state);
  if (stateIt != qTable.end()) {
    auto actionIt = stateIt->second.find(action);
    if (actionIt != stateIt->second.end()) {
        return actionIt->second;
    }
  }
  return 0.0;
}

bool QLearningAgent::saveQTable(std::string_view filePath) const {
  std::ofstream file(std::string(filePath), std::ios::out | std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << "[QLearningAgent] Error: No se pudo abrir el archivo para guardar: " << filePath << "\n";
    return false;
  }

  for (const auto& [state, actionMap] : qTable) {
    for (const auto& [action, qValue] : actionMap) {
      file << static_cast<int>(state.playerDistance)        << " "
           << static_cast<int>(state.playerAngle)           << " "
           << state.allyNearby                              << " "
           << static_cast<int>(state.predictedPlayerAction) << " "
           << static_cast<int>(action)                      << " "
           << qValue                                        << "\n";
      }
  }

  file.close();
  return true;
}

bool QLearningAgent::loadQTable(std::string_view filePath) {
  std::ifstream file(std::string(filePath), std::ios::in);
  if (!file.is_open()) {
    std::cerr << "[QLearningAgent] Advertencia: No se encontro Q-Table previa en: " << filePath << ". Se iniciara con tabla vacia.\n";
    return false;
  }

  qTable.clear();
  size_t fileDist, fileAngle, ally, predAct, act;
  double qValue;
  while (file >> fileDist >> fileAngle >> ally >> predAct >> act >> qValue) {
    egocentricState state;
    state.playerDistance        = static_cast<distanceSector>(fileDist);
    state.playerAngle           = static_cast<angleSector>(fileAngle);
    state.allyNearby            = static_cast<bool>(ally);
    state.predictedPlayerAction = static_cast<playerAction>(predAct);

    doppelAction actionEnum = static_cast<doppelAction>(act);
    qTable[state][actionEnum] = qValue;
  }

  file.close();
  return true;
}
