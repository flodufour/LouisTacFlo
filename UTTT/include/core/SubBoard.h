#pragma once

#include "core/Cell.h"
#include <array>
#include <stack>


/**
 * @class SubBoard
 * @brief Represents a single 3x3 Tic Tac Toe board within the Ultimate Tic Tac Toe grid.
 *
 * Handles local game logic including move placement, win detection,
 * and undo functionality.
 */
class SubBoard {
private:
    std::array<Cell, 9> _cells;
    bool _isFull = false;
    CellState _winner = CellState::EMPTY;
    int _filledCount = 0;

public:
    /**
     * @brief Constructs an empty sub-board.
     */
    SubBoard();

    /**
     * @brief Plays a move on the board.
     * @param index The cell index (0–8).
     * @param player The player making the move (X or O).
     * @return True if the move is valid and applied, false otherwise.
     */
    bool playMove(int index, CellState player);

    /**
     * @brief Checks if a player has won the sub-board.
     * @return CellState::X, CellState::O, or CellState::EMPTY if there is no winner.
     */
    CellState checkWinner() const;

    /**
     * @brief Checks if the board is full.
     * @return True if no empty cells remain.
     */
    bool isFull() const;

    /**
     * @brief Checks if the board is still playable.
     * @return True if there is no winner and the board is not full.
     */
    bool isPlayable() const;

    /**
     * @brief Gets a cell at a given index.
     * @param index The cell index (0–8).
     * @return Constant reference to the cell.
     */
    const Cell& getCell(int index) const;

    /**
     * @brief Resets the board to an empty state.
     */
    void reset();

    /**
     * @brief Checks if the board is completely empty.
     * @return True if all cells are EMPTY.
     */
    bool isEmpty() const;

    /**
     * @brief Undoes a move on a subboard by setting the cell at index to empty.
     * @param index The cell index (0–8) to revert.
     */
    void undoMove(int index);

    /**
     * @brief Checks the number of moves left on this subboard.
     * @return The number of empty cells on the subboard.
     */
    int getMovesLetftSubBoard() const;
};
