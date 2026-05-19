#pragma once

#include "ai/evaluate/IEvaluator.h"
#include "core/GameState.h"

class MCTSFeatureEvaluator : public IEvaluator
{
public:
    MCTSFeatureEvaluator();
    virtual ~MCTSFeatureEvaluator() override = default;

    // Interface IEvaluator standard (évalue selon le joueur du tour actuel)
    virtual int evaluate(const GameState& state) const override;

    // Surcharge spécifique MCTS appelée par MCTSStrategy
    int evaluate(const GameState& state, CellState perspective) const;

private:
    struct Features
    {
        int terminalWin = 0;
        int terminalLoss = 0;

        int metaOwned = 0;
        int metaOpponentOwned = 0;

        int metaTwoInRow = 0;
        int metaOneInRow = 0;
        int metaOpponentTwoInRow = 0;
        int metaOpponentOneInRow = 0;

        int metaFork = 0;
        int metaOpponentFork = 0;

        int subCellControl = 0;
        int subCellOpponentControl = 0;

        int subTwoInRow = 0;
        int subOneInRow = 0;
        int subOpponentTwoInRow = 0;
        int subOpponentOneInRow = 0;

        int subFork = 0;
        int subOpponentFork = 0;

        int forcedGood = 0;
        int forcedVeryGood = 0;
        int forcedBad = 0;
        int forcedVeryBad = 0;

        int freeMove = 0;

        int metaImportanceGood = 0;
        int metaImportanceVeryGood = 0;
        int metaImportanceBad = 0;
        int metaImportanceVeryBad = 0;

        int boardPositionBonus = 0;

        int metaNearWin = 0;
        int metaOpponentNearWin = 0;
    };

    struct Weights
    {
        int terminalWin;
        int terminalLoss;
        int metaOwned;
        int metaOpponentOwned;
        int metaTwoInRow;
        int metaOneInRow;
        int metaOpponentTwoInRow;
        int metaOpponentOneInRow;
        int metaFork;
        int metaOpponentFork;
        int subCellControl;
        int subCellOpponentControl;
        int subTwoInRow;
        int subOneInRow;
        int subOpponentTwoInRow;
        int subOpponentOneInRow;
        int subFork;
        int subOpponentFork;
        int forcedGood;
        int forcedVeryGood;
        int forcedBad;
        int forcedVeryBad;
        int freeMove;
        int metaImportanceGood;
        int metaImportanceVeryGood;
        int metaImportanceBad;
        int metaImportanceVeryBad;
        int boardPositionBonus;
        int metaNearWin;
        int metaOpponentNearWin;
    };

    // Tes poids originaux stricts conservés
    static constexpr Weights w{
        1000000,  -1000000, // Terminal (Win / Loss)
        1200,     -1200,    // Meta ownership
        8000,      1200,    // Meta threats
       -8000,     -1200,    // Meta opponent threats
        10000,    -10000,   // Meta forks
        2,        -2,       // Sub-board positional control
        120,       20,      // Sub-board threats
       -120,      -20,      // Sub-board opponent threats
        250,      -250,     // Sub forks
        180,                // forcedOffensive
        120,                // forcedDefensive
       -120,
       -180,                // forcedDanger
       -700,                 // Free move
        300,      600,     // Meta importance pressure
        -300,      -600,
        1,                  // Multiplicateur pour boardPositionBonus
        300,      -300      // Meta Near Win
    };

    static constexpr int boardWeight[9] = {
        3, 2, 3,
        2, 4, 2,
        3, 2, 3
    };

    // Extraction découplée basée uniquement sur l'acteur du tour ('me') et son opposant ('opp')
    Features extract(const UltimateBoard& b, CellState me, CellState opp) const;

    void extractTerminal(const UltimateBoard& b, Features& f, CellState me, CellState opp) const;
    void extractMeta(const UltimateBoard& b, Features& f, CellState me, CellState opp) const;
    void extractSubBoards(const UltimateBoard& b, Features& f, CellState me, CellState opp) const;
    void extractForcedMoves(const UltimateBoard& b, Features& f, CellState me, CellState opp) const;
    void extractMetaImportance(const UltimateBoard& b, int boardIndex, CellState me, CellState opp, Features& f) const;

    int dot(const Features& f) const;
};
