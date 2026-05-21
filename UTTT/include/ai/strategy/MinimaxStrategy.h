#pragma once

#include "core/AIMove.h"
#include "core/GameState.h"
#include "ai/strategy/IStrategy.h"
#include "ai/evaluate/IEvaluator.h"

#include <vector>
#include <cstdint>
#include <cstddef>

class MinimaxStrategy : public IStrategy {
public:
    // 1. Définition des types de nœuds pour l'élagage Alpha-Beta
    enum class TTFlag { EXACT, LOWER_BOUND, UPPER_BOUND };

    // 2. Structure de la table de transposition
    struct TTEntry {
        uint64_t key = 0;
        int value = 0;
        int depth = -1;
        TTFlag flag = TTFlag::EXACT;
        AIMove bestMove;
        bool wasMaximizing = false;
    };

    MinimaxStrategy(IEvaluator* evaluator, int depth = 3);

    AIMove chooseMove(GameState& state) override;

   void reset() override {
    for (auto& entry : _transpositionTable) {
        entry.key = 0;
    }

}

private:
    void orderMovesWithEval(GameState& state, std::vector<AIMove>& moves, const AIMove& ttHint, bool maximizing, int depth);

    int minimax(
        GameState& state,
        int depth,
        bool maximizing,
        int alpha,
        int beta
    );

    IEvaluator* _evaluatorLight;
    IEvaluator* _evaluator;
    int _maxDepth;

    static constexpr size_t TT_SIZE = 1 << 20;
    std::vector<TTEntry> _transpositionTable;


};
