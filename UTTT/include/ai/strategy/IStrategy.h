#pragma once

#include "core/AIMove.h"
#include "core/GameState.h"



/// @class IStrategy
/// @brief Interface for AI move selection strategies.
///
/// This interface defines a common contract for all AI strategies
class IStrategy {
public:
    /// @brief Selects the best move for the given game state.
    /// @param state Current game state.
    /// @return Chosen move for the AI.
    virtual AIMove chooseMove(GameState& state) = 0;

    /// @brief Virtual destructor for safe polymorphic deletion.
    virtual ~IStrategy() = default;

    /// @brief Reset for the strategy.
    virtual void reset() {}
};
