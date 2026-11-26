#pragma once

#include <string_view>

#include "MarkovPredictor.h"
#include "QLearning.h"
#include "agentTypes.h"

inline constexpr char DEFAULT_MARKOV_PATH[] = "./data/agentData/markov_data.txt";
inline constexpr char DEFAULT_QTABLE_PATH[] = "./data/agentData/qtable_data.txt";

struct AgentKnowledge {
  MarkovPredictor markov;
  QLearningAgent qAgent{0.1, 0.9, 0.15};  // alpha = 0.1, gamma = 0.9, epsilon = 0.15

  void notifyPlayerUpgrade() {
    qAgent.onPlayerUpgrade();
  }

  bool save(std::string_view markovPath = DEFAULT_MARKOV_PATH,
            std::string_view qTablePath = DEFAULT_QTABLE_PATH) const {
    if (!markov.saveFrequencies(markovPath)) return false;
    if (!qAgent.saveQTable(qTablePath)) return false;
    return true;
  }

  bool load(std::string_view markovPath = DEFAULT_MARKOV_PATH,
            std::string_view qTablePath = DEFAULT_QTABLE_PATH) {
    if (!qAgent.loadQTable(qTablePath)) return false;
    if (!markov.loadFrequencies(markovPath)) return false;
    qAgent.setEpsilon(0.10);
    return true;
  }
};

class Agent {
  private:
    AgentKnowledge& knowledge;

    egocentricState lastState;
    doppelAction    lastAction;
    bool            hasPreviousState;

    playerContext   lastPlayerContext;
    bool            hasPreviousContext;

    playerContext buildPlayerContext(const egocentricPerception& perception) const;

    egocentricState buildEgocentricState(const egocentricPerception& perception,
                                         playerAction predictedAction) const;

    doppelAngledAction calculateAngledAction(doppelAction action,
                                             const egocentricPerception& perception) const;

  public:
    explicit Agent(AgentKnowledge& sharedKnowledge);

    doppelAngledAction update(const egocentricPerception& perception);

    void applyReward(double reward, const egocentricPerception& newPerception);

    void resetTracking();

    inline void notifyPlayerUpgrade() {
      knowledge.notifyPlayerUpgrade();
    }

    bool saveKnowledge(std::string_view markovPath = DEFAULT_MARKOV_PATH,
                       std::string_view qTablePath = DEFAULT_QTABLE_PATH) const;

    bool loadKnowledge(std::string_view markovPath = DEFAULT_MARKOV_PATH,
                       std::string_view qTablePath = DEFAULT_QTABLE_PATH);
};
