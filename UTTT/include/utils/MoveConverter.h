#pragma once

#include "main.h"
#include "core/AIMove.h"

/**
 * @class MoveConverter
 * @brief Utility class to convert move coordinates between game engine and AI format.
 */
class MoveConverter {
public:
    /**
     * @brief Converts an AI move into a standard game move.
     * @param m The AI move to convert.
     * @return The converted GameMove structure.
     */
    GameMove toGameMove(AIMove m);

    /**
     * @brief Converts a standard game move into an AI move.
     * @param p The game move to convert.
     * @return The converted AIMove structure.
     */
    AIMove toAIMove(GameMove p);
};
