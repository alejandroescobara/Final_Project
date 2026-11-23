#pragma once

#include <cstddef>
#include <map>
#include <string_view>

#include "agentTypes.h"

class MarkovPredictor {
private:
    std::map<playerContext, std::map<playerAction, size_t>> frequencyTable;

    playerAction lastPlayerAction;

public:
    MarkovPredictor();

    void registerTransition(const playerContext& context, playerAction executedAction);

    playerAction predictNextAction(const playerContext& currentContext) const;

    double getProbability(const playerContext& context, playerAction action) const;

    // Métodos con std::string_view para evitar la creación de std::string temporales
    bool saveFrequencies(std::string_view filePath) const;
    bool loadFrequencies(std::string_view filePath);
};
