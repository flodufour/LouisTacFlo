#include "ai/GameManager.h"
#include "ai/strategy/MinimaxStrategy.h"
#include "ai/evaluate/FeatureEvaluator.h"
#include <iostream>

int GameManager::s_gameId = 0;

GameManager::GameManager(long long runTimestamp)
{
    _runTimestamp = runTimestamp;

    _evaluator = std::make_unique<FeatureEvaluator>();

    _minimaxStrategy = std::make_unique<MinimaxStrategy>(_evaluator.get(), 15);

 }

void GameManager::init(CellState mySide)
{
    if (_minimaxStrategy) {
        _minimaxStrategy->reset();
    }
    _gameId = s_gameId++;
    _me = mySide;
    _opponent = (mySide == CellState::X) ? CellState::O : CellState::X;

    _state.reset();
    _state.setPlayers(mySide);
}


void GameManager::finalizeGame()
{
    CellState w = _state.getWinner();
}

void GameManager::applyMove(const AIMove& move)
{
    _state.applyMove(move);
}

AIMove GameManager::chooseMove()
{
    return _minimaxStrategy->chooseMove(_state);
}


const GameState& GameManager::getState() const
{
    return _state;
}

CellState GameManager::getOpponent() const {
    return _opponent;
}

GameManager::~GameManager()
{
}
