#include "ai/evaluate/FeatureEvaluator.h"
#include "core/WinPatterns.h"
#include <algorithm>
#include <iostream>

int FeatureEvaluator::evaluate(const GameState& state) const
{
    Features f = extract(state);
    return dot(f);
}

FeatureEvaluator::Features
FeatureEvaluator::extract(const GameState& state) const
{
    Features f;

    const UltimateBoard& b = state.getBoard();

    CellState me = state.getMyPlayer();
    CellState opp = (me == CellState::X)
        ? CellState::O
        : CellState::X;

    extractTerminal(b, f, me, opp);

    if (f.terminalWin || f.terminalLoss)
        return f;

    extractMeta(b, f, me, opp);
    extractSubBoards(b, f, me, opp);
    extractForcedMoves(b, f, me, opp);
    return f;
}

void FeatureEvaluator::extractTerminal(
    const UltimateBoard& b,
    Features& f,
    CellState me,
    CellState opp) const
{
    CellState winner = b.checkWinner();

    if (winner == me)
        f.terminalWin = 1;
    else if (winner == opp)
        f.terminalLoss = 1;
}

void FeatureEvaluator::extractMeta(
    const UltimateBoard& b,
    Features& f,
    CellState me,
    CellState opp) const
{
    int myThreats = 0;
    int oppThreats = 0;

    for (int i = 0; i < 9; i++)
{
    const SubBoard& sb = b.getBoard(i);
    CellState owner = sb.checkWinner();

    if (owner == me)
    {
        f.metaOwned += boardWeight[i];
        continue;
    }
    else if (owner == opp)
    {
        f.metaOpponentOwned += boardWeight[i];
        continue;
    }

    if (sb.isFull())
        continue;

    int myLocalThreats = 0;
    int oppLocalThreats = 0;

    for (const auto& line : WIN_LINES)
    {
        int myCount = 0;
        int oppCount = 0;
        int emptyCount = 0;

        for (int idx : line)
        {
            CellState cell = sb.getCell(idx).getState();

            if (cell == me)
                myCount++;
            else if (cell == opp)
                oppCount++;
            else
                emptyCount++;
        }

        if (myCount == 2 && emptyCount == 1)
            myLocalThreats++;

        if (oppCount == 2 && emptyCount == 1)
            oppLocalThreats++;
    }

    // ICI exactement
    f.metaNearWin += myLocalThreats * boardWeight[i];
    f.metaOpponentNearWin += oppLocalThreats * boardWeight[i];
}

    for (const auto& line : WIN_LINES)
    {
        int myCount = 0;
        int oppCount = 0;
        int emptyCount = 0;

        for (int idx : line)
        {
            CellState owner = b.getBoard(idx).checkWinner();

            if (owner == me) myCount++;
            else if (owner == opp) oppCount++;
            else emptyCount++;
        }

        if (myCount > 0 && oppCount > 0)
            continue;

        if (myCount == 2 && emptyCount == 1)
        {
            f.metaTwoInRow++;
            myThreats++;
        }
        else if (myCount == 1 && emptyCount == 2)
        {
            f.metaOneInRow++;
        }

        if (oppCount == 2 && emptyCount == 1)
        {
            f.metaOpponentTwoInRow++;
            oppThreats++;
        }
        else if (oppCount == 1 && emptyCount == 2)
        {
            f.metaOpponentOneInRow++;
        }
    }

    if (myThreats >= 2)
        f.metaFork++;

    if (oppThreats >= 2)
        f.metaOpponentFork++;


}

void FeatureEvaluator::extractSubBoards(
    const UltimateBoard& b,
    Features& f,
    CellState me,
    CellState opp) const
{
    for (int boardIdx = 0; boardIdx < 9; boardIdx++)
    {
        const SubBoard& sb = b.getBoard(boardIdx);

        if (sb.checkWinner() != CellState::EMPTY || sb.isFull())
            continue;

        int myThreats = 0;
        int oppThreats = 0;

        for (int i = 0; i < 9; i++)
        {
            CellState owner = sb.getCell(i).getState();

            if (owner == me)
                f.subCellControl += boardWeight[i];
            else if (owner == opp)
                f.subCellOpponentControl += boardWeight[i];
        }

        for (const auto& line : WIN_LINES)
        {
            int myCount = 0;
            int oppCount = 0;
            int emptyCount = 0;

            for (int idx : line)
            {
                CellState owner = sb.getCell(idx).getState();

                if (owner == me) myCount++;
                else if (owner == opp) oppCount++;
                else emptyCount++;
            }

            if (myCount > 0 && oppCount > 0)
                continue;

            if (myCount == 2 && emptyCount == 1)
            {
                f.subTwoInRow++;
                myThreats++;
            }
            else if (myCount == 1 && emptyCount == 2)
            {
                f.subOneInRow++;
            }

            if (oppCount == 2 && emptyCount == 1)
            {
                f.subOpponentTwoInRow++;
                oppThreats++;
            }
            else if (oppCount == 1 && emptyCount == 2)
            {
                f.subOpponentOneInRow++;
            }
        }

        if (myThreats >= 2)
            f.subFork++;

        if (oppThreats >= 2)
            f.subOpponentFork++;
    }
}

void FeatureEvaluator::extractForcedMoves(
    const UltimateBoard& b,
    Features& f,
    CellState me,
    CellState opp) const
{
    int boardIndex = b.getActiveBoard();

    if (boardIndex == -1)
    {
        f.freeMove = 1;
        return;
    }

    const SubBoard& sb = b.getBoard(boardIndex);

    if (sb.checkWinner() != CellState::EMPTY || sb.isFull())
    {
        f.freeMove = 1;
        return;
    }

    evaluateMetaImportance(
        b,
        boardIndex,
        me,
        opp,
        f
    );

    for (const auto& line : WIN_LINES)
    {
        int myCount = 0;
        int oppCount = 0;
        int emptyCount = 0;

        for (int idx : line)
        {
            CellState owner = sb.getCell(idx).getState();

            if (owner == me) myCount++;
            else if (owner == opp) oppCount++;
            else emptyCount++;
        }


        if (myCount == 2 && emptyCount == 1)
            f.forcedGood++;
        else if (myCount == 1 && emptyCount == 2)
            f.forcedVeryGood++;

        else if (oppCount == 2 && emptyCount == 1)
            f.forcedVeryBad++;
        else if (oppCount == 1 && emptyCount == 2)
            f.forcedBad++;
    }

    f.boardPositionBonus -= boardWeight[boardIndex];
}

int FeatureEvaluator::evaluateMetaImportance(
    const UltimateBoard& b,
    int boardIndex,
    CellState me,
    CellState opp,
    Features& f) const
{
    for (const auto& line : WIN_LINES)
    {
        bool containsBoard = false;

        for (int idx : line)
        {
            if (idx == boardIndex)
            {
                containsBoard = true;
                break;
            }
        }

        if (!containsBoard)
            continue;

        int myCount = 0;
        int oppCount = 0;

        for (int idx : line)
        {
            if (idx == boardIndex)
                continue;

            CellState owner = b.getBoard(idx).checkWinner();

            if (owner == me)
                myCount++;
            else if (owner == opp)
                oppCount++;
        }



        if (myCount == 2)
            f.metaImportanceGood++;

        if (oppCount == 2)
            f.metaImportanceBad++;
    }

    return 0;
}

int FeatureEvaluator::dot(const Features& f) const
{
    int score = 0;

    score += f.terminalWin * w.terminalWin;
    score += f.terminalLoss * w.terminalLoss;

    score += f.metaOwned * w.metaOwned;
    score += f.metaOpponentOwned * w.metaOpponentOwned;

    score += f.metaTwoInRow * w.metaTwoInRow;
    score += f.metaOneInRow * w.metaOneInRow;
    score += f.metaOpponentTwoInRow * w.metaOpponentTwoInRow;
    score += f.metaOpponentOneInRow * w.metaOpponentOneInRow;

    score += f.metaFork * w.metaFork;
    score += f.metaOpponentFork * w.metaOpponentFork;

    score += f.subCellControl * w.subCellControl;
    score += f.subCellOpponentControl * w.subCellOpponentControl;

    score += f.subTwoInRow * w.subTwoInRow;
    score += f.subOneInRow * w.subOneInRow;
    score += f.subOpponentTwoInRow * w.subOpponentTwoInRow;
    score += f.subOpponentOneInRow * w.subOpponentOneInRow;

    score += f.subFork * w.subFork;
    score += f.subOpponentFork * w.subOpponentFork;

    score += f.forcedGood * w.forcedGood;
    score += f.forcedVeryGood * w.forcedVeryGood;
    score += f.forcedBad * w.forcedBad;
    score += f.forcedVeryBad * w.forcedVeryBad;

    score += f.freeMove * w.freeMove;

    score += f.metaImportanceGood * w.metaImportanceGood;
    score += f.metaImportanceBad * w.metaImportanceBad;

    score += f.boardPositionBonus * w.boardPositionBonus;

    score += f.metaNearWin * w.metaNearWin;
    score += f.metaOpponentNearWin * w.metaOpponentNearWin;



    return score;
}
