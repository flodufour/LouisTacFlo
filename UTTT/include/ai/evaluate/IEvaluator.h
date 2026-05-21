#pragma once
#include "core/GameState.h"

/**
 * @interface IEvaluator
 * @brief Engine-safe evaluation interface.
 *
 * Contract:
 * - MUST NOT modify GameState
 * - MUST be deterministic
 *
 * Convention:
 * - Positive score = advantageous for the current player
 * - Negative score = advantageous for the opponent
 */

class IEvaluator {
public:

    /**
     * @brief Evaluates a game state (read-only).
     * @param state The current game state to evaluate.
     * @return A heuristic score relative to the root player.
     */
    virtual int evaluate(const GameState& state) const = 0;

    /**
     * @brief Virtual destructor for safe polymorphic deletion.
     */
    virtual ~IEvaluator() = default;
};
