#include "core/UltimateBoard.h"
#include "core/WinPatterns.h"

UltimateBoard::UltimateBoard() {
    reset();
}

void UltimateBoard::reset() {
    for (auto& b : _boards) {
        b.reset();
    }
    _activeBoard = -1;
}

bool UltimateBoard::isValidMove(AIMove aIMove) const {
    int boardIndex = aIMove.boardIndex;
    int cellIndex = aIMove.cellIndex;
    if (boardIndex < 0 || boardIndex >= 9) return false;
    if (cellIndex < 0 || cellIndex >= 9) return false;

    const SubBoard& board = _boards[boardIndex];

    if (!board.isPlayable()) return false;

    if (!board.getCell(cellIndex).isEmpty()) return false;

    if (_activeBoard != -1 && boardIndex != _activeBoard)
        return false;

    return true;
}

bool UltimateBoard::playMove(AIMove aIMove, CellState player) {
    int boardIndex = aIMove.boardIndex;
    int cellIndex = aIMove.cellIndex;
    if (!isValidMove(aIMove)){
        return false;
    }

    _boards[boardIndex].playMove(cellIndex, player);

    updateActiveBoard(cellIndex);

    return true;
}

void UltimateBoard::updateActiveBoard(int lastCellIndex) {


    if (lastCellIndex < 0 || lastCellIndex >= 9) {
        _activeBoard = -1;
        return;
    }

    if (!_boards[lastCellIndex].isPlayable()) {
        _activeBoard = -1;
    }
    else {
        _activeBoard = lastCellIndex;
    }
}

CellState UltimateBoard::checkWinner() const {

    for (const auto& line : WIN_LINES) {
        CellState a = _boards[line[0]].checkWinner();
        CellState b = _boards[line[1]].checkWinner();
        CellState c = _boards[line[2]].checkWinner();

        if (a != CellState::EMPTY && a == b && b == c)
            return a;
    }

    if (!isFull())
        return CellState::EMPTY;

    int x = 0;
    int o = 0;

    for (int i = 0; i < 9; i++)
    {
        CellState w = _boards[i].checkWinner();

        if (w == CellState::X)
            x++;
        else if (w == CellState::O)
            o++;
    }

    if (x > o)
        return CellState::X;

    if (o > x)
        return CellState::O;

    return CellState::EMPTY;
}

bool UltimateBoard::isFull() const {
    for (const auto& b : _boards) {
        if (!b.isFull())
            return false;
    }
    return true;
}

int UltimateBoard::getActiveBoard() const {
    return _activeBoard;
}

SubBoard& UltimateBoard::getBoard(int index) {
    return _boards[index];
}

const SubBoard& UltimateBoard::getBoard(int index) const {
    return _boards[index];
}

bool UltimateBoard::isEmpty() const
{
    for (int i = 0; i < 9; i++)
    {
        if (!getBoard(i).isEmpty())
            return false;
    }
    return true;
}

void UltimateBoard::undoMove(const AIMove& move, int prevActiveBoard)
{
    _boards[move.boardIndex].undoMove(move.cellIndex);
    _activeBoard = prevActiveBoard;
}

int UltimateBoard::getMovesLeftBoard() const{

    int count = 0;

    for (int i = 0;i < 9; i++)
    {
        count += _boards[i].getMovesLetftSubBoard();
    }

    return count;
}


