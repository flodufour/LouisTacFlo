#include "ai/strategy/MinimaxStrategy.h"
#include <limits>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <chrono>

MinimaxStrategy::MinimaxStrategy(IEvaluator* evaluator, IEvaluator* evaluatorLight, int depth)
    : _evaluator(evaluator), _evaluatorLight(evaluatorLight), _maxDepth(depth)
{
    _transpositionTable.resize(TT_SIZE);
}

AIMove MinimaxStrategy::chooseMove(GameState& state) {
    auto start = std::chrono::high_resolution_clock::now();
    AIMove globalBestMove;

    int remainingMoves = state.getMovesLeft();
    int effectiveMaxDepth = std::min(_maxDepth, remainingMoves);

    for (int d = 1; d <= effectiveMaxDepth && (std::chrono::high_resolution_clock::now() - start) < std::chrono::milliseconds(200); ++d) {
        std::cout << d << std::endl;
        int alpha = -9999999;
        int beta  =  9999999;

        auto moves = state.getValidMoves();

        uint64_t h = state.getHash();
        TTEntry& entry = _transpositionTable[h & (TT_SIZE - 1)];

        AIMove hint;
        if (entry.key == h && entry.wasMaximizing == true) {
            hint = entry.bestMove;
        }

        orderMovesWithEval(state, moves, hint, true, 1);
        AIMove bestMoveIteration;
        int bestScoreIteration = -9999999;

        for (const auto& move : moves) {
            auto undo = state.applyMoveFast(move);
            int score = minimax(state, d - 1, false, alpha, beta);
            state.undoMove(undo);

            if (score > bestScoreIteration) {
                bestScoreIteration = score;
                bestMoveIteration = move;
            }
            alpha = std::max(alpha, bestScoreIteration);
        }

        globalBestMove = bestMoveIteration;

        if (bestScoreIteration > 800000) break;
    }

    return globalBestMove;
}

int MinimaxStrategy::minimax(GameState& state, int depth, bool maximizing, int alpha, int beta)
{
    uint64_t hash = state.getHash();
    int alphaOrig = alpha;

    TTEntry& entry = _transpositionTable[hash & (TT_SIZE - 1)];

    if (entry.key == hash && entry.depth >= depth && entry.wasMaximizing == maximizing) {
        if (entry.flag == TTFlag::EXACT) return entry.value;
        else if (entry.flag == TTFlag::LOWER_BOUND) alpha = std::max(alpha, entry.value);
        else if (entry.flag == TTFlag::UPPER_BOUND) beta = std::min(beta, entry.value);

        if (alpha >= beta) return entry.value;
    }

    if (state.isTerminal() || depth == 0)
        return _evaluator->evaluate(state);

    auto moves = state.getValidMoves();
    if (moves.empty()) return _evaluator->evaluate(state);

    AIMove hint;
    if (entry.key == hash && entry.wasMaximizing == maximizing) {
        hint = entry.bestMove;
    }

    orderMovesWithEval(state, moves, hint, maximizing, depth);
    int best;
    AIMove bestMoveLocal;

    if (maximizing) {
        best = -9999999;
        for (const auto& move : moves) {
            auto undo = state.applyMoveFast(move);
            int score = minimax(state, depth - 1, false, alpha, beta);
            state.undoMove(undo);

            if (score > best) {
                best = score;
                bestMoveLocal = move;
            }
            alpha = std::max(alpha, best);
            if (beta <= alpha) break;
        }
    } else {
        best = 9999999;
        for (const auto& move : moves) {
            auto undo = state.applyMoveFast(move);
            int score = minimax(state, depth - 1, true, alpha, beta);
            state.undoMove(undo);

            if (score < best) {
                best = score;
                bestMoveLocal = move;
            }
            beta = std::min(beta, best);
            if (beta <= alpha) break;
        }
    }


if (entry.key != hash && depth < entry.depth) {
} else {
    entry.key = hash;
    entry.value = best;
    entry.depth = depth;
    entry.bestMove = bestMoveLocal;
    entry.wasMaximizing = maximizing;

    if (best <= alphaOrig) entry.flag = TTFlag::UPPER_BOUND;
    else if (best >= beta) entry.flag = TTFlag::LOWER_BOUND;
    else entry.flag = TTFlag::EXACT;
}

    return best;
}

struct ScoredMove {
    AIMove move;
    int score;
};

void MinimaxStrategy::orderMovesWithEval(GameState& state, std::vector<AIMove>& moves, const AIMove& ttHint, bool maximizing, int depth) {
    if (moves.size() <= 1) return;

    std::vector<ScoredMove> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (const auto& move : moves) {
        int score = 0;

        if (move == ttHint) {
            score = maximizing ? 9999999 : -9999999;
        } else {
            auto undo = state.applyMoveFast(move);
            score = _evaluatorLight->evaluate(state);
            state.undoMove(undo);
        }

        scoredMoves.push_back({move, score});
    }

    if (maximizing) {
        std::sort(scoredMoves.begin(), scoredMoves.end(), [](const ScoredMove& a, const ScoredMove& b) {
            return a.score > b.score;
        });
    } else {
        std::sort(scoredMoves.begin(), scoredMoves.end(), [](const ScoredMove& a, const ScoredMove& b) {
            return a.score < b.score;
        });
    }

    for (size_t i = 0; i < moves.size(); ++i) {
        moves[i] = scoredMoves[i].move;
    }
}
