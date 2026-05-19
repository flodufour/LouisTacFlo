#include "ai/strategy/MCTSStrategy.h"
#include "ai/evaluate/MCTSFeatureEvaluator.h" // Indispensable pour le dynamic_cast de perspective
#include <limits>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <chrono>

MCTSStrategy::MCTSStrategy(IEvaluator* lightEvaluator, double explorationConst, int maxTimeMs)
    : _lightEvaluator(lightEvaluator), _explorationConst(explorationConst), _maxTimeMs(maxTimeMs)
{
    _rng.seed(1337);
}

double MCTSStrategy::normalizeScore(int rawScore) const {
    // Normalisation douce adaptée à l'échelle de tes poids miroirs MCTS (~ -1000 à +1000)
    double scaling = 400.0;
    return 1.0 / (1.0 + std::exp(-static_cast<double>(rawScore) / scaling));
}

AIMove MCTSStrategy::chooseMove(GameState& state) {
    auto startTime = std::chrono::high_resolution_clock::now();
    CellState aiPlayer = state.getCurrentPlayer();

    // Création de la racine : représente l'état actuel, c'est à l'IA de jouer
    auto root = std::make_unique<MCTSNode>(AIMove{-1, -1}, nullptr, aiPlayer);
    root->unvisitedMoves = state.getValidMoves();

    // Si un seul coup est possible, on l'exécute immédiatement (économie de temps CPU)
    if (root->unvisitedMoves.size() == 1) {
        return root->unvisitedMoves[0];
    }

    int iterations = 0;
    int maxDepthReached = 0;

    // Vecteur de rollback alloué une seule fois pour éviter les ralentissements liés aux allocations dynamiques
    std::vector<MoveUndo> undoStack;
    undoStack.reserve(81);

    while (true) {
        // Contrôle du temps strict toutes les 64 itérations
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
            // Tentative de récupération de la surcharge symétrique par perspective de ton nouvel évaluateur
            auto* mctsEval = dynamic_cast<MCTSFeatureEvaluator*>(_lightEvaluator);
            int rawScore = 0;

            if (mctsEval != nullptr) {
                // FORCE l'évaluation du point de vue invariant de ton IA racine
                rawScore = mctsEval->evaluate(state, aiPlayer);
            } else {
                // Sécurité si l'interface pointe vers un autre type d'évaluateur
                rawScore = _lightEvaluator->evaluate(state);
            }

            score = normalizeScore(rawScore);
        }

        // 4. BACKPROPAGATION
        backpropagate(expandedNode, score);

        // BACKTRACKING : On nettoie et réinitialise l'état du plateau pour l'itération suivante
        while (!undoStack.empty()) {
            state.undoMove(undoStack.back());
            undoStack.pop_back();
        }

        iterations++;
    }

    std::cout << "[MCTS_EVAL] Iterations executees : " << iterations
              << " | Profondeur max de l'arbre : " << maxDepthReached << std::endl;

    // Sélection robuste basée sur le nœud ayant reçu le plus grand nombre de visites
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

    // Extraction en O(1)
    node->unvisitedMoves[index] = node->unvisitedMoves.back();
    node->unvisitedMoves.pop_back();

    if (node->unvisitedMoves.empty()) {
        node->isFullyExpanded = true;
    }

    // Alternance stricte des joueurs au fil de la descente de l'arbre virtuel
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
        node->wins += score; // Le score étant figé sur la perspective absolue de l'IA, on l'additionne partout
        node = node->parent;
    }
}

double MCTSStrategy::getUCB1(const MCTSNode* node, const MCTSNode* child) const {
    if (child->visits == 0.0) {
        return std::numeric_limits<double>::infinity();
    }

    double exploitation = child->wins / child->visits;

    // Récupération de l'identifiant de notre IA en remontant à la racine
    const MCTSNode* root = node;
    while (root->parent != nullptr) {
        root = root->parent;
    }
    CellState aiPlayer = root->playerToMove;

    // Logique Minimax intégrée à l'UCT :
    // Si le nœud actuel appartient à l'adversaire (node->playerToMove != aiPlayer),
    // l'adversaire cherche à minimiser nos gains. Sa valeur d'exploitation est donc (1.0 - la nôtre).
    if (node->playerToMove != aiPlayer) {
        exploitation = 1.0 - exploitation;
    }

    double exploration = _explorationConst * std::sqrt(std::log(node->visits) / child->visits);
    return exploitation + exploration;
}
