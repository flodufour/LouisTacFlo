#include "ai/GameManager.h"
#include "ai/ArenaHost.h"
#include "utils/ZobristHasher.h"
#include <ctime>
#include <iostream>

int main() {
    Zobrist::init();
    long long start_timestamp = std::time(nullptr);

    GameManager manager(start_timestamp);
    ArenaHost arena(manager);

    arena.runSession(100, Level::MEDIUM_2);

    std::cerr << "MAIN FINISHED" << std::endl;
    return 0;
}
