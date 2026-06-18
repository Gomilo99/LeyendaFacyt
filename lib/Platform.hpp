#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <cstdlib>
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <locale.h>
#endif

/**
 * @namespace Platform
 * @brief Abstracción multiplataforma para terminal e input.
 *
 * Unifica APIs de Windows (Win32 Console API, _getch) y Linux/POSIX
 * (termios raw mode, ioctl TIOCGWINSZ, read) detrás de funciones
 * inline simples. Todo el código del juego usa este namespace en
 * lugar de #ifdef dispersos.
 *
 * Uso en main.cpp:
 * @code
 *   Platform::initTerminal();
 *   atexit(Platform::restoreTerminal);
 * @endcode
 */
namespace Platform {

#ifndef _WIN32
static termios orig_termios;
#endif

/**
 * @brief Inicializa la terminal para el juego.
 *
 * En Windows: habilita secuencias ANSI (VT processing) y UTF-8.
 * En Linux: activa raw mode (ECHO+ICANON off, VMIN=1) y locale UTF-8.
 * El estado original del terminal se guarda para restoreTerminal().
 * Debe llamarse una vez al inicio de main(), antes de cualquier input.
 */
inline void initTerminal() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode))
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(CP_UTF8);
#else
    setlocale(LC_ALL, "");
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
#endif
}

/**
 * @brief Restaura la terminal a su estado original.
 *
 * En Linux revierte los cambios de tcsetattr hechos en initTerminal();
 * en Windows es no-op porque el cambio de modo de consola persiste
 * solo durante la vida del proceso. Debe llamarse via atexit() para
 * garantizar restauración incluso si el programa termina abruptamente.
 */
inline void restoreTerminal() {
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
#endif
}

/**
 * @brief Obtiene el ancho actual de la terminal en caracteres.
 * @return Número de columnas, o 80 si no se puede determinar.
 *
 * En Windows usa GetConsoleScreenBufferInfo().srWindow.
 * En Linux usa ioctl TIOCGWINSZ, luego getenv("COLUMNS"), luego 80.
 */
inline int getTerminalWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return 80;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
        return w.ws_col;
    char* col = getenv("COLUMNS");
    if (col) { int n = atoi(col); if (n > 0) return n; }
    return 80;
#endif
}

/**
 * @brief Obtiene la altura actual de la terminal en caracteres.
 * @return Número de filas, o 24 si no se puede determinar.
 *
 * En Windows usa GetConsoleScreenBufferInfo().srWindow.
 * En Linux usa ioctl TIOCGWINSZ, luego getenv("LINES"), luego 24.
 */
inline int getTerminalHeight() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return 24;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_row > 0)
        return w.ws_row;
    char* row = getenv("LINES");
    if (row) { int n = atoi(row); if (n > 0) return n; }
    return 24;
#endif
}

/**
 * @brief Lee una tecla sin esperar Enter ni hacer eco.
 * @return El carácter ASCII leído, o 0 si no hay datos disponibles.
 *
 * En Windows usa _getch() (conio.h). En Linux usa read() sobre
 * STDIN_FILENO con raw mode activo (requiere initTerminal() previo).
 * No bloquea más allá de la siguiente pulsación de tecla.
 */
inline char getKey() {
#ifdef _WIN32
    return char(_getch());
#else
    char c;
    if (read(STDIN_FILENO, &c, 1) > 0)
        return c;
    return 0;
#endif
}

/**
 * @brief Reactiva el echo de teclas en la terminal.
 *
 * Solo tiene efecto en Linux, donde el raw mode de initTerminal()
 * desactiva ECHO. Se usa antes de std::getline() para que el
 * usuario vea lo que escribe (ej. nombre del jugador).
 * En Windows es no-op porque std::cin maneja echo nativamente.
 */
inline void echoOn() {
#ifndef _WIN32
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
#endif
}

/**
 * @brief Desactiva el echo de teclas en la terminal.
 *
 * Solo tiene efecto en Linux. Complemento de echoOn(): se llama
 * después de std::getline() para volver al modo sin echo del juego.
 * En Windows es no-op.
 */
inline void echoOff() {
#ifndef _WIN32
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
#endif
}

} // namespace Platform
#endif