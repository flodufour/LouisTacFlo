#pragma once

#include "ai/GameManager.h"
#include "utils/MoveConverter.h"
#include "main.h"

/**
 * @class ArenaHost
 * @brief Organizes communication between the AI and the Arena engine.
 *
 * Handles the high-level game loop, player side detection, and
 * ensures synchronization between the engine protocol and the GameManager.
 */
class ArenaHost {
public:
    /**
     * @brief Constructs an ArenaHost linked to a specific manager.
     * @param manager Reference to the AI manager responsible for strategy.
     */
    ArenaHost(GameManager& manager);

    /**
     * @brief Runs a complete session of multiple games.
     * @param numGames Number of games to play.
     * @param level Difficulty level for the engine.
     */
    void runSession(int numGames, Level level);

private:
    /**
     * @brief Executes the logic for a single game from start to finish.
     */
    void playSingleGame();

    GameManager& _manager;
    MoveConverter _converter;
};
