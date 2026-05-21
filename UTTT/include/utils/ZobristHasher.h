#pragma once
#include <cstdint>

/**
 * @class Zobrist
 * @brief Handles Zobrist hashing keys initialization for state tracking.
 * * Provides static pseudorandom keys used to incrementally hash the game state
 * (board pieces, active boards, and player turns) for efficient transposition tables.
 */

class Zobrist
{
public:
    /**
     * @brief Initializes all random 64-bit keys for cells, active boards, and players.
     * @note Uses a fixed seed (deterministic generation) for consistency across runs.
     */
    static void init();

    /**
     * @brief Random keys for each cell on the board.
     * Dimensions: [board_index][cell_index][player_id] where 0 = X, 1 = O.
     */
    static uint64_t cell[9][9][2];

    /**
     * @brief Random keys representing the currently active micro-board.
     * Index 0-8 represent specific boards, while index 9 represents a free-move state (-1).
     */
    static uint64_t activeBoard[10];

    /**
     * @brief Random keys for the current player's turn.
     * Index 0 = X's turn, 1 = O's turn.
     */
    static uint64_t player[2];

    /**
     * @brief Safely maps an active board index (-1 to 8) to a valid array index (0 to 9).
     * @param b The active board index (where -1 means all boards are active).
     * @return An integer index from 0 to 9.
     */
    static int activeBoardIndex(int b);
};
