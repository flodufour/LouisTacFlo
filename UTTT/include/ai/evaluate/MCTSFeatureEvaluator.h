#pragma once

#include "ai/evaluate/IEvaluator.h"
#include "core/GameState.h"

class MCTSFeatureEvaluator : public IEvaluator
{
public:
    MCTSFeatureEvaluator();
    virtual ~MCTSFeatureEvaluator() override = default;

    // Interface IEvaluator standard (utilisée par défaut)
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

        // Features de la case forcée (dynamiquement incrémentées / décrémentées)
        int forcedOffensive = 0;
        int forcedDefensive = 0;
        int forcedDanger = 0;

        int freeMove = 0;

        int metaImportanceGood = 0;
        int metaImportanceBad = 0;

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
        int forcedOffensive;
        int forcedDefensive;
        int forcedDanger;
        int freeMove;
        int metaImportanceGood;
        int metaImportanceBad;
        int boardPositionBonus;
        int metaNearWin;
        int metaOpponentNearWin;
    };

    // Grille de poids miroirs (Zero-Sum) calibrée pour l'IA racine
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
        180,                // forcedOffensive (+ si IA, - si adversaire via le code)
        120,                // forcedDefensive (+ si IA, - si adversaire via le code)
       -150,                // forcedDanger    (- si IA, + si adversaire via le code)
       -70,               // Free move
        500,      -500,     // Meta importance pressure
        1,                  // Multiplicateur pour boardPositionBonus
        600,      -600      // Meta Near Win
    };

    static constexpr int boardWeight[9] = {
        3, 2, 3,
        2, 4, 2,
        3, 2, 3
    };

    Features extract(const UltimateBoard& b, CellState me, CellState opp, CellState turnPlayer) const;

    void extractTerminal(const UltimateBoard& b, Features& f, CellState me, CellState opp) const;
    void extractMeta(const UltimateBoard& b, Features& f, CellState me, CellState opp) const;
    void extractSubBoards(const UltimateBoard& b, Features& f, CellState me, CellState opp) const;
    void extractForcedMoves(const UltimateBoard& b, Features& f, CellState me, CellState opp, CellState turnPlayer) const;
    void extractMetaImportance(const UltimateBoard& b, int boardIndex, CellState me, CellState opp, Features& f) const;

    int dot(const Features& f) const;
};
