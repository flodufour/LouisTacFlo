#pragma once

#include "subBoard.h"
#include "AIMove.h"
#include <array>
#include <stack>

/**
 * @class UltimateBoard
 * @brief Represents the full 9x9 Ultimate Tic Tac Toe game board.
 *
 * The board is composed of a 3x3 grid of SubBoards (9 total) and tracks
 * game rules such as macro-win conditions and active board restrictions.
 */
class UltimateBoard {
private:
    std::array<SubBoard, 9> _boards;

    int _activeBoard;

public:
    /**
     * @brief Constructs an empty Ultimate board.
     */
    UltimateBoard();

    /**
     * @brief Plays a move on the board.
     * @param aIMove The move to apply.
     * @param player The player making the move (X or O).
     * @return True if the move was successfully applied.
     */
    bool playMove(AIMove aIMove, CellState player);

    /**
     * @brief Checks if a move is valid in the current state.
     * @param aIMove The move to validate.
     * @return True if the move is legal.
     */
    bool isValidMove(AIMove aIMove) const;

    /**
     * @brief Checks if a player has won the entire game.
     * @return CellState::X, CellState::O, or CellState::EMPTY if there is no winner.
     */
    CellState checkWinner() const;

    /**
     * @brief Checks if the entire board is a draw.
     * @return True if all boards are full.
     */
    bool isFull() const;

    /**
     * @brief Updates the active board based on the last move played.
     * @param lastCellIndex Index of the last played cell inside the sub-board.
     */
     void updateActiveBoard(int lastCellIndex);

    /**
     * @brief Gets the current active board index.
     * @return The active board index, or -1 if any board is playable (free-move).
     */
    int getActiveBoard() const;

    /**
     * @brief Gets a modifiable sub-board.
     * @param index Sub-board index [0..8].
     * @return Reference to the sub-board.
     */
    SubBoard& getBoard(int index);

    /**
     * @brief Gets a read-only sub-board.
     * @param index Sub-board index [0..8].
     * @return Const reference to the sub-board.
     */
    const SubBoard& getBoard(int index) const;

    /**
     * @brief Resets the entire game state to clean/empty.
     */
    void reset();

    /**
     * @brief Checks if the board is completely empty.
     * @return True if no moves have been played.
     */
    bool isEmpty() const;

    /**
     * @brief Undoes a specified AIMove on the board.
     * @param move The AIMove to be undone.
     * @param prevActiveBoard The active board index that was active before this move was played.
     */
    void undoMove(const AIMove& move, int prevActiveBoard);

    /**
     * @brief Checks the number of moves left across the entire board.
     * @return The total number of empty cells left in the game.
     */
    int getMovesLeftBoard() const;


};
