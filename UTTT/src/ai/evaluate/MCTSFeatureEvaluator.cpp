#include "ai/evaluate/MCTSFeatureEvaluator.h"
#include "core/WinPatterns.h"

MCTSFeatureEvaluator::MCTSFeatureEvaluator() {}

int MCTSFeatureEvaluator::evaluate(const GameState& state) const
{
    // Mode standard : Évalue la grille selon le joueur du tour actuel
    return evaluate(state, state.getCurrentPlayer());
}

int MCTSFeatureEvaluator::evaluate(const GameState& state, CellState perspective) const
{
    const UltimateBoard& b = state.getBoard();
    CellState me = perspective; // L'IA racine de la recherche
    CellState opp = (me == CellState::X) ? CellState::O : CellState::X;

    // turnPlayer est indispensable pour savoir à qui est le tour à ce nœud précis de l'arbre
    Features f = extract(b, me, opp, state.getCurrentPlayer());
    return dot(f);
}

MCTSFeatureEvaluator::Features MCTSFeatureEvaluator::extract(const UltimateBoard& b, CellState me, CellState opp, CellState turnPlayer) const
{
    Features f;
    extractTerminal(b, f, me, opp);

    if (f.terminalWin || f.terminalLoss)
        return f;

    extractMeta(b, f, me, opp);
    extractSubBoards(b, f, me, opp);
    extractForcedMoves(b, f, me, opp, turnPlayer);
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

    for (const auto& line : WIN_LINES)
    {
        int myCount = 0; int oppCount = 0; int emptyCount = 0;
        for (int idx : line)
        {
            CellState owner = b.getBoard(idx).checkWinner();
            if (owner == me)       myCount++;
            else if (owner == opp) oppCount++;
            else                   emptyCount++;
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

void MCTSFeatureEvaluator::extractForcedMoves(const UltimateBoard& b, Features& f, CellState me, CellState opp, CellState turnPlayer) const
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

    // Détermination de qui est actif à ce niveau de l'arbre et de son opposant direct
    CellState active = turnPlayer;
    CellState inactive = (active == CellState::X) ? CellState::O : CellState::X;

    for (const auto& line : WIN_LINES)
    {
        int activeCount = 0; int inactiveCount = 0; int emptyCount = 0;
        for (int idx : line)
        {
            CellState owner = sb.getCell(idx).getState();
            if (owner == active)        activeCount++;
            else if (owner == inactive) inactiveCount++;
            else                        emptyCount++;
        }

        // TACTIQUE SYMETRIQUE :
        // Si le joueur actif a 2 pions, il peut clore la macro-case.
        // Si cet actif est l'IA, le score grimpe (+), si c'est l'adversaire, le score baisse (-)
        if (activeCount == 2 && emptyCount == 1)
        {
            if (active == me) f.forcedOffensive++;
            else              f.forcedOffensive--;
        }

        // Si le joueur inactif a 2 pions, le joueur actif a l'opportunité cruciale de bloquer.
        if (inactiveCount == 2 && emptyCount == 1)
        {
            if (active == me) f.forcedDefensive++;
            else              f.forcedDefensive--;
        }

        // Si le joueur inactif possède un pion installé sur la ligne, le joueur actif subit la pression.
        if (inactiveCount == 1 && emptyCount == 2)
        {
            if (active == me) f.forcedDanger++;
            else              f.forcedDanger--;
        }
    }

    // Le bonus lié à la contrainte géométrique s'inverse lui aussi selon le joueur actif
    if (active == me) {
        f.boardPositionBonus -= boardWeight[boardIndex];
    } else {
        f.boardPositionBonus += boardWeight[boardIndex];
    }
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

        if (myCount == 2)  f.metaImportanceGood++;
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

    score += f.forcedOffensive * w.forcedOffensive;
    score += f.forcedDefensive * w.forcedDefensive;
    score += f.forcedDanger * w.forcedDanger;

    score += f.freeMove * w.freeMove;
    score += f.metaImportanceGood * w.metaImportanceGood;
    score += f.metaImportanceBad * w.metaImportanceBad;
    score += f.boardPositionBonus * w.boardPositionBonus;
    score += f.metaNearWin * w.metaNearWin;
    score += f.metaOpponentNearWin * w.metaOpponentNearWin;
    return score;
}
