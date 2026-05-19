#include "ai/evaluate/MCTSFeatureEvaluator.h"
#include "core/WinPatterns.h"

MCTSFeatureEvaluator::MCTSFeatureEvaluator() {}

int MCTSFeatureEvaluator::evaluate(const GameState& state) const
{
    return evaluate(state, state.getCurrentPlayer());
}

int MCTSFeatureEvaluator::evaluate(const GameState& state, CellState perspective) const
{
    const UltimateBoard& b = state.getBoard();

    CellState turnPlayer = state.getCurrentPlayer();
    CellState oppPlayer = (turnPlayer == CellState::X) ? CellState::O : CellState::X;

    Features f = extract(b, turnPlayer, oppPlayer);
    int rawScore = dot(f);

    if (turnPlayer == perspective)
    {
        return rawScore;
    }
    else
    {
        return -rawScore;
    }
}

MCTSFeatureEvaluator::Features MCTSFeatureEvaluator::extract(const UltimateBoard& b, CellState me, CellState opp) const
{
    Features f;
    extractTerminal(b, f, me, opp);

    if (f.terminalWin || f.terminalLoss)
        return f;

    extractMeta(b, f, me, opp);
    extractSubBoards(b, f, me, opp);
    extractForcedMoves(b, f, me, opp);
    return f;
}

void MCTSFeatureEvaluator::extractTerminal(const UltimateBoard& b, Features& f, CellState me, CellState opp) const
{
    CellState winner = b.checkWinner();
    if (winner == me)       f.terminalWin = 1;
    else if (winner == opp) f.terminalLoss = 1;
}

void MCTSFeatureEvaluator::extractMeta(const UltimateBoard& b, Features& f, CellState me, CellState opp) const
{
    int myThreats = 0;
    int oppThreats = 0;

    // 1. ANALYSE STATIQUE DES PROPRIÉTÉS ET DES COMPTEURS LOCAUX (NEAR WIN)
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

        // Si la sous-grille est pleine (match nul), elle n'apporte aucun point de structure locale (Near Win)
        if (sb.isFull())
            continue;

        int myLocalThreats = 0;
        int oppLocalThreats = 0;

        for (const auto& line : WIN_LINES)
        {
            int myCount = 0; int oppCount = 0; int emptyCount = 0;
            for (int idx : line)
            {
                CellState cell = sb.getCell(idx).getState();
                if (cell == me)       myCount++;
                else if (cell == opp) oppCount++;
                else                  emptyCount++;
            }
            if (myCount == 2 && emptyCount == 1)  myLocalThreats++;
            if (oppCount == 2 && emptyCount == 1) oppLocalThreats++;
        }

        f.metaNearWin += myLocalThreats * boardWeight[i];
        f.metaOpponentNearWin += oppLocalThreats * boardWeight[i];
    }

    // 2. ANALYSE DYNAMIQUE DES ALIGNEMENTS MACRO (PROTÉGÉE CONTRE LES ÉGALITÉS)
    for (const auto& line : WIN_LINES)
    {
        int myCount = 0; int oppCount = 0; int emptyCount = 0;
        bool lineIsBlocked = false;

        for (int idx : line)
        {
            const SubBoard& sb = b.getBoard(idx);
            CellState owner = sb.checkWinner();

            // PROTECTION STRICTE : Si la case est une égalité locale (EMPTY et FULL), la ligne macro est morte
            if (owner == CellState::EMPTY && sb.isFull())
            {
                lineIsBlocked = true;
                break;
            }

            if (owner == me)       myCount++;
            else if (owner == opp) oppCount++;
            else                   emptyCount++;
        }

        // Si la ligne macro est condamnée par un match nul local, on l'oublie immédiatement
        if (lineIsBlocked)
            continue;

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

    if (myThreats >= 2)  f.metaFork++;
    if (oppThreats >= 2) f.metaOpponentFork++;
}
void MCTSFeatureEvaluator::extractSubBoards(const UltimateBoard& b, Features& f, CellState me, CellState opp) const
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
            if (owner == me)       f.subCellControl += boardWeight[i];
            else if (owner == opp) f.subCellOpponentControl += boardWeight[i];
        }

        for (const auto& line : WIN_LINES)
        {
            int myCount = 0; int oppCount = 0; int emptyCount = 0;
            for (int idx : line)
            {
                CellState owner = sb.getCell(idx).getState();
                if (owner == me)       myCount++;
                else if (owner == opp) oppCount++;
                else                   emptyCount++;
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

        if (myThreats >= 2)  f.subFork++;
        if (oppThreats >= 2) f.subOpponentFork++;
    }
}

void MCTSFeatureEvaluator::extractForcedMoves(const UltimateBoard& b, Features& f, CellState me, CellState opp) const
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

    extractMetaImportance(b, boardIndex, me, opp, f);

    for (const auto& line : WIN_LINES)
    {
        int myCount = 0; int oppCount = 0; int emptyCount = 0;
        for (int idx : line)
        {
            CellState owner = sb.getCell(idx).getState();
            if (owner == me)       myCount++;
            else if (owner == opp) oppCount++;
            else                   emptyCount++;
        }

        if(emptyCount == 3) f.forcedGood ++;
        if (myCount == 1 && oppCount == 1)       f.forcedVeryGood++;
        if (oppCount == 2 && emptyCount == 1)      f.forcedVeryBad++;
        if (oppCount == 1 && emptyCount == 2)      f.forcedBad++;
    }

    f.boardPositionBonus -= boardWeight[boardIndex];
}

void MCTSFeatureEvaluator::extractMetaImportance(const UltimateBoard& b, int boardIndex, CellState me, CellState opp, Features& f) const
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

        int myCount = 0; int oppCount = 0;
        for (int idx : line)
        {
            if (idx == boardIndex)
                continue;

            CellState owner = b.getBoard(idx).checkWinner();
            if (owner == me)       myCount++;
            else if (owner == opp) oppCount++;
        }
        if (myCount == 1)  f.metaImportanceGood++;
        if (myCount == 0)  f.metaImportanceBad++;
        if (myCount == 2)  f.metaImportanceBad++;
        if (oppCount == 2) f.metaImportanceBad++;
    }
}

int MCTSFeatureEvaluator::dot(const Features& f) const
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
