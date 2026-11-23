#pragma once

#include <string_view>

#include "agentTypes.h"
#include "MarkovPredictor.h"
#include "QLearning.h"

inline constexpr char DEFAULT_MARKOV_PATH[] = "./data/agentData/markov_data.txt";
inline constexpr char DEFAULT_QTABLE_PATH[] = "./data/agentData/qtable_data.txt";

class Agent {
  private:
    MarkovPredictor markov;
    QLearningAgent  qAgent;

    egocentricState lastState;
    doppelAction    lastAction;
    bool            hasPreviousState;

    playerContext buildPlayerContext(const egocentricPerception& perception) const;

    egocentricState buildEgocentricState(const egocentricPerception& perception, 
                                         playerAction predictedAction) const;

    doppelAngledAction calculateAngledAction(doppelAction action, 
                                             const egocentricPerception& perception) const;

  public:
    Agent();

    doppelAngledAction update(const egocentricPerception& perception);

    void applyReward(double reward, const egocentricPerception& newPerception);

    inline void notifyPlayerUpgrade() {
      qAgent.onPlayerUpgrade();
    }

    bool saveKnowledge(std::string_view markovPath = DEFAULT_MARKOV_PATH,
                       std::string_view qTablePath = DEFAULT_QTABLE_PATH) const;

    bool loadKnowledge(std::string_view markovPath = DEFAULT_MARKOV_PATH,
                       std::string_view qTablePath = DEFAULT_QTABLE_PATH);
};
