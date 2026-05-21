#pragma once

#include "core/GameState.h"
#include "main.h"
#include "ai/strategy/IStrategy.h"
#include "ai/evaluate/IEvaluator.h"
#include <memory>


/**
 * @class GameManager
 * @brief Central controller for managing game sessions and AI logic.
 *
 * Orchestrates the game flow, coordinates between the evaluation engine,
 * the search strategy, and handles data logging for training purposes.
 */
class GameManager
{
private:

    static int s_gameId;
    int _gameId;

    long long _runTimestamp;

    GameState _state;
    CellState _me;
    CellState _opponent;
    std::unique_ptr<IEvaluator> _evaluator;
    std::unique_ptr<IStrategy> _minimaxStrategy;

public:

    /**
     * @brief Constructs a GameManager.
     * @param _runTimestamp Shared timestamp for identifying this execution run.
     */
    GameManager(long long _runTimestamp);

    /**
     * @brief Initializes the manager with the assigned player side.
     * @param mySide The side (X or O) assigned to this AI instance.
     */
    void init(CellState mySide);

    /**
     * @brief Finalizes game records.
     */
    void finalizeGame();

    /**
     * @brief Applies a move to the internal board state and logs it.
     * @param move The AIMove to be executed.
     */
    void applyMove(const AIMove& move);

    /**
     * @brief Selects the best move using the current strategy.
     * @return The AIMove chosen by the AI.
     */
    AIMove chooseMove();

    /**
     * @brief Gets the current game state.
     * @return Constant reference to the internal GameState.
     */
    const GameState& getState() const;

    /**
     * @brief Gets the opponent's side.
     * @return CellState representing the opponent.
     */
    CellState getOpponent() const;

    /**
     * @brief Destroys the GameManager.
     */
    ~GameManager();

};
