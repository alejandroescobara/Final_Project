#pragma once

#include <map>
#include <string_view>
#include <random>
#include <algorithm>

#include "agentTypes.h"

class QLearningAgent {
private:
    std::map<egocentricState, std::map<doppelAction, double>> qTable;
    double alpha;         // Tasa de aprendizaje
    double gamma;         // Factor de descuento
    double epsilon;       // Tasa de exploración actual
    
    // Parámetros de Decaimiento
    double minEpsilon;    // Límite mínimo (0.02 = 2%)
    double decayFactor;   // Factor de decaimiento por paso (ej. 0.997)

    std::mt19937 gen;
    std::uniform_real_distribution<double> dist;
    std::uniform_int_distribution<size_t> actionDist;

public:
    QLearningAgent(double learningRate = 0.1, double discountFactor = 0.9, double explorationRate = 0.15);

    doppelAction selectAction(const egocentricState& state);
    void updateQValue(const egocentricState& lastState,
                      doppelAction actionTaken, 
                      double reward, 
                      const egocentricState& newState);
    
    double getQValue(const egocentricState& state, doppelAction action) const;

    inline void setEpsilon(double newEpsilon) { 
      epsilon = std::clamp(newEpsilon, 0.0, 1.0); 
    }

    inline void resetMatchEpsilon(bool isFirstRun) { 
      epsilon = isFirstRun ? 0.15 : 0.10; 
    }

    inline void onPlayerUpgrade() { 
      epsilon = 0.10; 
    }

    inline void stepDecay() { 
      if (epsilon > minEpsilon) {
        epsilon = std::max(minEpsilon, epsilon * decayFactor);
      }
    }

    bool saveQTable(std::string_view filePath) const;
    bool loadQTable(std::string_view filePath);
};
