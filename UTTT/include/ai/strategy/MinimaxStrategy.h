#pragma once

#include "core/AIMove.h"
#include "core/GameState.h"
#include "ai/strategy/IStrategy.h"
#include "ai/evaluate/IEvaluator.h"

#include <vector>
#include <cstdint>
#include <cstddef>

/**
 * @class MinimaxStrategy
 * @brief AI Strategy implementing Minimax with Alpha-Beta pruning, Iterative Deepening, and a Transposition Table.
 */
class MinimaxStrategy : public IStrategy {
public:
    /**
     * @enum TTFlag
     * @brief Indicates the accuracy of the value stored in a Transposition Table entry.
     */
    enum class TTFlag { EXACT, LOWER_BOUND, UPPER_BOUND };

    /**
     * @struct TTEntry
     * @brief A single slot entry in the Transposition Table for caching evaluated states.
     */
    struct TTEntry {
        uint64_t key = 0;
        int value = 0;
        int depth = -1;
        TTFlag flag = TTFlag::EXACT;
        AIMove bestMove;
        bool wasMaximizing = false;
    };

    /**
     * @brief Constructs a Minimax strategy instance.
     * @param evaluator Pointer to the state evaluator engine.
     * @param depth The target maximum lookahead depth (defaults to 3).
     */
    MinimaxStrategy(IEvaluator* evaluator, int depth = 3);

    /**
     * @brief Calculates and selects the best move using iterative deepening.
     * @param state The current game state.
     * @return The chosen AIMove.
     */
    AIMove chooseMove(GameState& state) override;

    /**
     * @brief Clears the Transposition Table by invalidating all entries.
     */
   void reset() override {
    for (auto& entry : _transpositionTable) {
        entry.key = 0;
    }

}

private:
    /**
     * @brief Sorts moves in-place to optimize Alpha-Beta pruning efficiency.
     * * Prioritizes the Transposition Table hint move first, followed by static
     * positional heuristics (center cell, then corners).
     * * @param state The current game state.
     * @param moves The collection of legal moves to sort.
     * @param ttHint Global/Local best move hint extracted from the Transposition Table.
     * @param maximizing Current player's optimization objective (true for Max, false for Min).
     * @param depth Current search depth remaining.
     */
    void orderMovesWithEval(GameState& state, std::vector<AIMove>& moves, const AIMove& ttHint, bool maximizing, int depth);

    /**
     * @brief Recursive Minimax algorithm function featuring Alpha-Beta pruning and TT lookups.
     * @param state The game state to analyze.
     * @param depth The current depth remaining in the branch exploration.
     * @param maximizing True if searching for the maximum score, false for the minimum score.
     * @param alpha The lower bound score window for pruning.
     * @param beta The upper bound score window for pruning.
     * @return The evaluated score of the state branch.
     */
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
