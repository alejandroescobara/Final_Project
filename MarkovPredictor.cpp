#include "MarkovPredictor.h"

#include <fstream>
#include <iostream>

MarkovPredictor::MarkovPredictor() 
               : lastPlayerAction(playerAction::INACTIVE) {}

void MarkovPredictor::registerTransition(const playerContext& context, playerAction executedAction) {
  frequencyTable[context][executedAction]++;
  lastPlayerAction = executedAction;
}

playerAction MarkovPredictor::predictNextAction(const playerContext& currentContext) const {
  auto contextIt = frequencyTable.find(currentContext);

  if (contextIt == frequencyTable.end() || contextIt->second.empty()) {
    return playerAction::INACTIVE;
  }

  playerAction mostLikelyAction = playerAction::INACTIVE;
  size_t maxFrequency = 0;

  for (const auto& [action, count] : contextIt->second) {
    if (count > maxFrequency) {
      maxFrequency = count;
      mostLikelyAction = action;
    }
  }

  return mostLikelyAction;
}

double MarkovPredictor::getProbability(const playerContext& context, playerAction action) const {
  auto contextIt = frequencyTable.find(context);

  if (contextIt == frequencyTable.end() || contextIt->second.empty()) {
    return 0.0;
  }

  size_t totalOccurrences = 0;
  size_t targetActionOccurrences = 0;

  for (const auto& [act, count] : contextIt->second) {
    totalOccurrences += count;
    if (act == action) {
      targetActionOccurrences = count;
    }
  }

  if (totalOccurrences == 0) return 0.0;

  return static_cast<double>(targetActionOccurrences) / static_cast<double>(totalOccurrences);
}

bool MarkovPredictor::saveFrequencies(std::string_view filePath) const {
  std::ofstream file(std::string(filePath), std::ios::out | std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << "[MarkovPredictor] Error: No se pudo abrir el archivo para guardar: " << filePath << "\n";
    return false;
  }

  for (const auto& [context, actionMap] : frequencyTable) {
    for (const auto& [action, count] : actionMap) {
      file << static_cast<int>(context.threatDistance) << " "
           << static_cast<int>(context.threatAngle)    << " "
           << context.isSurrounded                     << " "
           << static_cast<int>(context.lastAction)     << " "
           << static_cast<int>(action)                 << " "
           << count                                    << "\n";
    }
  }

  file.close();
  return true;
}

bool MarkovPredictor::loadFrequencies(std::string_view filePath) {
  std::ifstream file(std::string(filePath), std::ios::in);
  if (!file.is_open()) {
    std::cerr << "[MarkovPredictor] Advertencia: No se encontro archivo previo en: " << filePath << ". Se iniciara con tabla vacia.\n";
    return false;
  }

  frequencyTable.clear();

  size_t dist, angle, surr, lastAct, act, count;
  while (file >> dist >> angle >> surr >> lastAct >> act >> count) {
    playerContext ctx;
    ctx.threatDistance = static_cast<distanceSector>(dist);
    ctx.threatAngle    = static_cast<angleSector>(angle);
    ctx.isSurrounded   = static_cast<bool>(surr);
    ctx.lastAction     = static_cast<playerAction>(lastAct);

    playerAction actionEnum = static_cast<playerAction>(act);

    frequencyTable[ctx][actionEnum] = count;
  }

  file.close();
  return true;
}
