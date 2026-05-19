#pragma once

#include "ai/strategy/IStrategy.h"
#include "ai/evaluate/IEvaluator.h"
#include "core/GameState.h"
#include "core/AIMove.h"
#include <vector>
#include <memory>
#include <random>

class MCTSStrategy : public IStrategy {
public:
    /**
     * @param lightEvaluator Pointeur vers ton MCTSFeatureEvaluator (passé via l'interface IEvaluator)
     * @param explorationConst Constante UCT (1.414 par défaut)
     * @param maxTimeMs Temps maximum alloué en millisecondes (3000ms pour éviter le Timeout)
     */
    MCTSStrategy(IEvaluator* mCTSFeatureEvaluator, double explorationConst = 1.414, int maxTimeMs = 3000);
    virtual ~MCTSStrategy() override = default;

    virtual AIMove chooseMove(GameState& state) override;
    virtual void reset() override { _rng.seed(1337); }

private:
    struct MCTSNode {
        AIMove move;
        MCTSNode* parent = nullptr;
        std::vector<std::unique_ptr<MCTSNode>> children;

        double visits = 0.0;
        double wins = 0.0; // Accumule les scores normalisés (0.0 à 1.0) du point de vue de l'IA racine

        std::vector<AIMove> unvisitedMoves;
        CellState playerToMove; // Joueur qui doit prendre la décision DEPUIS ce nœud
        bool isFullyExpanded = false;

        MCTSNode(AIMove m, MCTSNode* p, CellState player)
            : move(m), parent(p), playerToMove(player) {}
    };

    MCTSNode* select(MCTSNode* node, GameState& state, std::vector<MoveUndo>& undoStack, int currentDepth, int& maxDepth);
    MCTSNode* expand(MCTSNode* node, GameState& state, std::vector<MoveUndo>& undoStack);
    void backpropagate(MCTSNode* node, double score);

    double getUCB1(const MCTSNode* node, const MCTSNode* child) const;
    double normalizeScore(int rawScore) const;

    IEvaluator* _mCTSFeatureEvaluator;
    double _explorationConst;
    int _maxTimeMs;
    std::mt19937 _rng;
};
