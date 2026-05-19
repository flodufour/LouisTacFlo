#include "ai/strategy/MCTSStrategy.h"
#include "ai/evaluate/MCTSFeatureEvaluator.h"
#include <limits>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <chrono>

MCTSStrategy::MCTSStrategy(IEvaluator* mCTSFeatureEvaluator, double explorationConst, int maxTimeMs)
    : _mCTSFeatureEvaluator(mCTSFeatureEvaluator), _explorationConst(explorationConst), _maxTimeMs(maxTimeMs)
{
    _rng.seed(1337);
}

double MCTSStrategy::normalizeScore(int rawScore) const {
    // Normalisation par Sigmoïde adaptée à l'échelle de l'évaluateur symétrique
    double scaling = 400.0;
    return 1.0 / (1.0 + std::exp(-static_cast<double>(rawScore) / scaling));
}

AIMove MCTSStrategy::chooseMove(GameState& state) {
    auto startTime = std::chrono::high_resolution_clock::now();
    CellState aiPlayer = state.getCurrentPlayer();

    auto root = std::make_unique<MCTSNode>(AIMove{-1, -1}, nullptr, aiPlayer);
    root->unvisitedMoves = state.getValidMoves();

    if (root->unvisitedMoves.size() == 1) {
        return root->unvisitedMoves[0];
    }

    int iterations = 0;
    int maxDepthReached = 0;

    std::vector<MoveUndo> undoStack;
    undoStack.reserve(81);

    while (true) {
        if ((iterations & 63) == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
            if (elapsed >= _maxTimeMs) {
                break;
            }
        }

        // 1. SELECTION
        MCTSNode* selectedNode = select(root.get(), state, undoStack, 0, maxDepthReached);
        MCTSNode* expandedNode = selectedNode;

        // 2. EXPANSION
        if (!state.isTerminal()) {
            expandedNode = expand(selectedNode, state, undoStack);
            maxDepthReached = std::max(maxDepthReached, static_cast<int>(undoStack.size()));
        }

        // 3. EVALUATION DIRECTE VIA LA PERSPECTIVE SYMETRIQUE
        double score = 0.5;
        if (state.isTerminal()) {
            CellState winner = state.getWinner();
            if (winner == aiPlayer) score = 1.0;
            else if (winner == CellState::EMPTY) score = 0.5;
            else score = 0.0;
        } else {
            auto* mctsEval = dynamic_cast<MCTSFeatureEvaluator*>(_mCTSFeatureEvaluator);
            int rawScore = 0;

            if (mctsEval != nullptr) {
                // L'évaluateur renvoie un score aligné sur l'IA racine
                rawScore = mctsEval->evaluate(state, aiPlayer);
            } else {
                rawScore = _mCTSFeatureEvaluator->evaluate(state);
            }

            score = normalizeScore(rawScore);
        }

        // 4. BACKPROPAGATION
        backpropagate(expandedNode, score);

        // BACKTRACKING
        while (!undoStack.empty()) {
            state.undoMove(undoStack.back());
            undoStack.pop_back();
        }

        iterations++;
    }

    std::cout << "[MCTS_EVAL] Iterations executees : " << iterations
              << " | Profondeur max de l'arbre : " << maxDepthReached << std::endl;

    MCTSNode* bestChild = nullptr;
    double maxVisits = -1.0;

    for (const auto& child : root->children) {
        if (child->visits > maxVisits) {
            maxVisits = child->visits;
            bestChild = child.get();
        }
    }

    if (bestChild != nullptr) {
        return bestChild->move;
    }

    return state.getValidMoves()[0];
}

MCTSStrategy::MCTSNode* MCTSStrategy::select(MCTSNode* node, GameState& state, std::vector<MoveUndo>& undoStack, int currentDepth, int& maxDepth) {
    while (node->isFullyExpanded && !node->children.empty()) {
        MCTSNode* bestChild = nullptr;
        double bestUCB = -std::numeric_limits<double>::infinity();

        for (const auto& child : node->children) {
            double ucb = getUCB1(node, child.get());
            if (ucb > bestUCB) {
                bestUCB = ucb;
                bestChild = child.get();
            }
        }

        node = bestChild;
        undoStack.push_back(state.applyMoveFast(node->move));
        currentDepth++;
        maxDepth = std::max(maxDepth, currentDepth);
    }
    return node;
}

MCTSStrategy::MCTSNode* MCTSStrategy::expand(MCTSNode* node, GameState& state, std::vector<MoveUndo>& undoStack) {
    if (node->unvisitedMoves.empty() && !node->isFullyExpanded) {
        node->unvisitedMoves = state.getValidMoves();
        if (node->unvisitedMoves.empty()) {
            node->isFullyExpanded = true;
            return node;
        }
    }

    std::uniform_int_distribution<size_t> dist(0, node->unvisitedMoves.size() - 1);
    size_t index = dist(_rng);
    AIMove move = node->unvisitedMoves[index];

    node->unvisitedMoves[index] = node->unvisitedMoves.back();
    node->unvisitedMoves.pop_back();

    if (node->unvisitedMoves.empty()) {
        node->isFullyExpanded = true;
    }

    CellState nextPlayer = (node->playerToMove == CellState::X) ? CellState::O : CellState::X;

    auto child = std::make_unique<MCTSNode>(move, node, nextPlayer);
    MCTSNode* childPtr = child.get();
    node->children.push_back(std::move(child));

    undoStack.push_back(state.applyMoveFast(move));
    return childPtr;
}

void MCTSStrategy::backpropagate(MCTSNode* node, double score) {
    while (node != nullptr) {
        node->visits += 1.0;
        node->wins += score; // Le score est invariant et absolu (du point de vue de l'IA racine)
        node = node->parent;
    }
}

double MCTSStrategy::getUCB1(const MCTSNode* node, const MCTSNode* child) const {
    if (child->visits == 0.0) {
        return std::numeric_limits<double>::infinity();
    }

    // Le score stocké est invariant (toujours du point de vue de l'IA)
    double exploitation = child->wins / child->visits;

    // Récupération de l'identité de notre IA (la racine)
    const MCTSNode* root = node;
    while (root->parent != nullptr) {
        root = root->parent;
    }
    CellState aiPlayer = root->playerToMove;

    // SÉLECTION MULTI-JOUEUR DE L'UCT :
    // Si la décision à ce nœud appartient à l'adversaire (node->playerToMove != aiPlayer),
    // l'adversaire veut MINIMISER notre score.
    // Pour choisir son meilleur coup, sa valeur d'exploitation est donc l'exact inverse de la nôtre.
    if (node->playerToMove != aiPlayer) {
        exploitation = 1.0 - exploitation;
    }

    double exploration = _explorationConst * std::sqrt(std::log(node->visits) / child->visits);
    return exploitation + exploration;
}
