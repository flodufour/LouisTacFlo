#include "core/AIMove.h"
#include <cstdint>

/**
 * @struct MoveUndo
 * @brief Stores the minimum required data to revert a played move.
 */
struct MoveUndo
{
    AIMove move;
    int prevActiveBoard;
};
