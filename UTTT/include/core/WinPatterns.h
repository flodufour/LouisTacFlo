#pragma once

/** @brief All possible winning lines in a 3x3 grid.
 *
 *  Each row represents a winning combination of cell indices.
 *  Used to check win conditions in both sub-boards and meta-board.
 */
static const int WIN_LINES[8][3] = {
    {0,1,2},{3,4,5},{6,7,8},
    {0,3,6},{1,4,7},{2,5,8},
    {0,4,8},{2,4,6}
};
