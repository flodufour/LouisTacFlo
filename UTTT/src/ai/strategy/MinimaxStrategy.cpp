#include "ai/strategy/MinimaxStrategy.h"
#include <limits>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <chrono>

MinimaxStrategy::MinimaxStrategy(IEvaluator* evaluator, int depth)
    : _evaluator(evaluator), _maxDepth(depth)
{
    _transpositionTable.resize(TT_SIZE);
}
static uint64_t totalNodes = 0;

AIMove MinimaxStrategy::chooseMove(GameState& state) {
    totalNodes = 0;
    auto start = std::chrono::high_resolution_clock::now();
    AIMove globalBestMove;

    int remainingMoves = state.getMovesLeft();
    int effectiveMaxDepth = std::min(_maxDepth, remainingMoves);

    for (int d = 1; d <= effectiveMaxDepth && (std::chrono::high_resolution_clock::now() - start) < std::chrono::milliseconds(100); ++d) {
        std::cout << "Current depth : " << d << std::endl;
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

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "\n==========================================" << std::endl;
    std::cout << "                   STATS                  " << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "->Duration     : " << duration << " ms" << std::endl;
    std::cout << "->Total Nodes       : " << totalNodes << std::endl;
    if (duration > 0) {
        std::cout << "-> Search Time : " << (totalNodes / duration) << " nodes/ms" << std::endl;
    } else {
        std::cout << "-> Search Time : To fast to be in ms." << std::endl;
    }
    std::cout << "==========================================\n" << std::endl;

    return globalBestMove;
}

int MinimaxStrategy::minimax(GameState& state, int depth, bool maximizing, int alpha, int beta)
{
    totalNodes++;
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

void MinimaxStrategy::orderMovesWithEval(GameState& state, std::vector<AIMove>& moves, const AIMove& ttHint, bool maximizing, int depth) {
    if (moves.size() <= 1) return;

    std::sort(moves.begin(), moves.end(), [&](const AIMove& a, const AIMove& b) {
        int scoreA = 0;
        int scoreB = 0;

        if (a == ttHint) scoreA += 100000;
        if (b == ttHint) scoreB += 100000;

        if (a.cellIndex == 4) scoreA += 50;
        if (b.cellIndex == 4) scoreB += 50;

        if (a.cellIndex == 0 || a.cellIndex == 2 || a.cellIndex == 6 || a.cellIndex == 8) scoreA += 20;
        if (b.cellIndex == 0 || b.cellIndex == 2 || b.cellIndex == 6 || b.cellIndex == 8) scoreB += 20;

        return scoreA > scoreB;
    });
}
