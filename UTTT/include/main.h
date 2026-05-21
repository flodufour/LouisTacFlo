
#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
#include <string>

/**
 * @enum Level
 * @brief Defines the difficulty levels available for the game engine.
 */
enum class Level {
    EASY_1,
    EASY_2,
    MEDIUM_1,
    MEDIUM_2,
    HARD_1,
    HARD_2,
    VERY_HARD_1,
    VERY_HARD_2,
};

/**
 * @enum Mode
 * @brief Execution mode of the application.
 */
enum class Mode {
    DEBUG,
    ARENA,
};

/**
 * @enum Winner
 * @brief Represents the end-game result status.
 */
enum Winner {
    NO_WINNER,
    IA,
    PLAYER,
    IA_AND_PLAYER,
};

/**
 * @struct GameMove
 * @brief Represents a move coordinates in standard row and column formats.
 */
struct GameMove {
    int row;
    int col;
};

/**
 * @struct IGame
 * @brief Interface providing the high-level API communication hook with the external core engine.
 */
struct IGame {
    virtual ~IGame() = default;
    virtual Winner getWinner() const = 0;
    virtual bool isFinish() = 0;
    virtual bool isAllGameFinish() const = 0;
    virtual bool getMove(GameMove& outMove) = 0;
    virtual void setMove(const GameMove& move) = 0;
    virtual void initialize(unsigned int nbGame, Level level, Mode mode, bool alwaysPlayFirst, const std::string& alias = "Player") = 0;
};




extern IGame& game;

#endif // MAIN_H_INCLUDED
