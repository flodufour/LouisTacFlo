#pragma once

#include "core/GameState.h"
#include "ai/evaluate/IEvaluator.h"


/**
 * @class FeatureEvaluator
 * @brief Evaluation engine that computes a static heuristic score for a GameState.
 *
 * This class extracts structural, tactical, and positional components (features)
 * from an Ultimate Tic-Tac-Toe board and computes a linear combination using a
 * weight matrix (dot product).
 */
class FeatureEvaluator : public IEvaluator
{
public:
    /**
     * @brief Computes the heuristic score from the current player's perspective.
     * @param state The current state of the game to evaluate.
     * @return An integer score representing the static evaluation.
     */
    int evaluate(const GameState& state) const override;

    /**
     * @brief Computes the heuristic score from a specific player's perspective.
     * @param state The current state of the game to evaluate.
     * @param perspective The player side (X or O) from whose viewpoint the score is computed.
     * @return An integer score representing the static evaluation.
     */
    int evaluate(const GameState& state, CellState perspective) const;

private:
    /**
     * @struct Features
     * @brief Holds quantitative metrics extracted from a specific board state.
     */
    struct Features
    {
        // Terminal
        int terminalWin = 0;
        int terminalLoss = 0;

        // Meta board control
        int metaOwned = 0;
        int metaOpponentOwned = 0;

        // Meta threats
        int metaTwoInRow = 0;
        int metaOneInRow = 0;
        int metaOpponentTwoInRow = 0;
        int metaOpponentOneInRow = 0;

        // Meta forks
        int metaFork = 0;
        int metaOpponentFork = 0;

        // Sub-board positional control
        int subCellControl = 0;
        int subCellOpponentControl = 0;

        // Sub-board threats
        int subTwoInRow = 0;
        int subOneInRow = 0;
        int subOpponentTwoInRow = 0;
        int subOpponentOneInRow = 0;

        // Sub forks
        int subFork = 0;
        int subOpponentFork = 0;

        // Forced move system
        int forcedGood = 0;
        int forcedVeryGood = 0;
        int forcedBad = 0;
        int forcedVeryBad = 0;

        // Free move
        int freeMove = 0;

        // Meta importance pressure
        int metaImportanceGood = 0;
        int metaImportanceBad = 0;

        // Positional board control
        int boardPositionBonus = 0;

        int metaNearWin = 0;
        int metaOpponentNearWin = 0;
    };

    /**
     * @struct Weights
     * @brief Coefficient matrix associated with each metric inside the Features struct.
     */
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
        int metaImportanceBad;

        int boardPositionBonus;

        int metaNearWin;
        int metaOpponentNearWin;
    };

    /** @brief Static baseline heuristic weight parameters. */
    static constexpr Weights w{
        // Terminal
         1000000,   // terminalWin
        -1000000,   // terminalLoss

        // Meta ownership
         1200,      // metaOwned
        -1500,      // metaOpponentOwned

        // Meta threats
         8000,      // metaTwoInRow
         1200,      // metaOneInRow
        -15000,     // metaOpponentTwoInRow
        -1800,      // metaOpponentOneInRow

        // Meta forks
         10000,     // metaFork
        -12000,     // metaOpponentFork

        // Sub-board positional control
         2,         // subCellControl
        -3,         // subCellOpponentControl

        // Sub-board threats
         120,       // subTwoInRow
         20,        // subOneInRow
        -180,       // subOpponentTwoInRow
        -35,        // subOpponentOneInRow

        // Sub forks
         250,       // subFork
        -350,       // subOpponentFork

        // Forced move system
         40,        // forcedGood
         180,       // forcedVeryGood
        -60,        // forcedBad
        -300,       // forcedVeryBad

        // Free move
        -1000,      // freeMove

        // Meta importance pressure
         500,       // metaImportanceGood
        -700,       // metaImportanceBad

        // Positional board control
         8,         // boardPositionBonus
         600,       // metaNearWin
        -850        // metaOpponentNearWin
    };
    /** @brief Standard 3x3 layout weight matrix prioritizing centers, corners, then edges. */
    static constexpr int boardWeight[9] =
    {
        3,2,3,
        2,4,2,
        3,2,3
    };


private:
    /**
     * @brief Orchestrates the extraction of all heuristic parameters from a game state.
     * @param state The game state to inspect.
     * @return An populated Features object containing all calculated raw values.
     */
    Features extract(const GameState& state) const;

    /**
     * @brief Evaluates terminal winning or losing scenarios at the macro board scale.
     */
    void extractTerminal(
        const UltimateBoard& b,
        Features& f,
        CellState me,
        CellState opp) const;

    /**
    * @brief Evaluates alignment patterns and macro ownership levels across won sub-boards.
    */
    void extractMeta(
        const UltimateBoard& b,
        Features& f,
        CellState me,
        CellState opp) const;

    /**
    * @brief Evaluates local cell layout structures inside individual incomplete sub-boards.
    */
    void extractSubBoards(
        const UltimateBoard& b,
        Features& f,
        CellState me,
        CellState opp) const;
    /**
    * @brief Computes tactical threat vectors related to controlling where the opponent is forced to play.
    */
    void extractForcedMoves(
        const UltimateBoard& b,
        Features& f,
        CellState me,
        CellState opp) const;

    /**
     * @brief Assesses the positional weight and criticality of a targeted sub-board.
     * @return 0 upon successful contextual assignment inside Features& f.
     */
    int evaluateMetaImportance(
        const UltimateBoard& b,
        int boardIndex,
        CellState me,
        CellState opp,
        Features& f) const;

    /**
     * @brief Computes the final matrix dot product between features and weight vectors.
     * @param f Structured features filled with raw observations.
     * @return The scalar dot product integer score.
     */
    int dot(const Features& f) const;
};
