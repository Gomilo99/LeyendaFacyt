#include "../lib/GameManager.hpp"
#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode))
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(CP_UTF8);
#endif
    GameManager game;
    game.run();
    return 0;
}
