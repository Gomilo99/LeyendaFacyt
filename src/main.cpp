#include "../lib/GameManager.hpp"
#include "../lib/Platform.hpp"
#include <cstdlib>

int main() {
    Platform::initTerminal();
    atexit(Platform::restoreTerminal);

    GameManager game;
    game.run();
    return 0;
}
