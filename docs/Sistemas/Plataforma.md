---
creado: 22/07/2026
modificado: 22/07/2026
tipo: Avance
tags: # deuda-tecnica, idea-loca, bug-critico, bug, refactor
titulo: Capa de Abstraccion Multiplataforma
proyecto: "[[LeyendaFacyt]]"
area: Sistemas
estado: En progreso
prioridad: 2
dificultad: Media
version: 1.0.0
---
# Capa de Abstracción Multiplataforma

> Archivos: `lib/Platform.hpp` (header-only), `Makefile`

`Platform.hpp` abstrae diferencias entre Windows (Win32 Console API) y Linux/POSIX (termios, ioctl) para terminal, input y detección de tamaño. El resto del código no necesita `#ifdef _WIN32`.

Ver también: [[Mapa]] (GameManager usa Platform para input), [[Combate]] (ScreenBuffer usa Platform para tamaño de terminal), [[Inventario]] (InventoryUI usa Platform para input).

---

## Platform.hpp — API

| Función | Windows | Linux |
|---------|---------|-------|
| `initTerminal()` | `SetConsoleMode(ENABLE_VT_PROCESSING)` + `SetConsoleOutputCP(CP_UTF8)` | `setlocale()` + raw mode termios |
| `restoreTerminal()` | no-op | `tcsetattr()` restaurando estado original |
| `getTerminalWidth()` | `GetConsoleScreenBufferInfo` | `ioctl(TIOCGWINSZ)` → `getenv("COLUMNS")` → 80 |
| `getTerminalHeight()` | `GetConsoleScreenBufferInfo` | `ioctl(TIOCGWINSZ)` → `getenv("LINES")` → 24 |
| `getKey()` | `_getch()` | `read()` con raw mode activo |
| `echoOn()` | no-op | `tcsetattr(ECHO |= ...)` |
| `echoOff()` | no-op | `tcsetattr(ECHO &= ~...)` |

---

## Raw mode en Linux

```cpp
struct termios raw;
tcgetattr(STDIN_FILENO, &orig_termios);  // guardar estado original
raw = orig_termios;
raw.c_lflag &= ~(ECHO | ICANON);         // sin echo, sin buffering de línea
raw.c_cc[VMIN] = 1;                      // read() retorna con 1 byte
raw.c_cc[VTIME] = 0;                     // sin timeout
tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // aplicar
```

- **ECHO off**: las teclas no se muestran (el juego tiene su propia UI)
- **ICANON off**: modo no-canónico — cada byte disponible inmediatamente
- **VMIN=1**: `read()` retorna con 1 byte disponible
- `orig_termios` guardado en `initTerminal()`, restaurado en `restoreTerminal()` via `atexit`

---

## getKey() cross-platform

```cpp
// Windows
return char(_getch());      // conio.h, no requiere Enter, sin echo

// Linux
char c;
read(STDIN_FILENO, &c, 1);  // raw mode activo, retorna inmediatamente
return c;
```

Ambos retornan el código ASCII sin mostrar el carácter ni requerir Enter. El menú de combate (W/S/SPACE), overworld (WASD) e inventario (W/S/A/D/SPACE/Q) responden igual en ambos SO.

---

## echoOn() / echoOff()

En Linux con raw mode, ECHO está desactivado globalmente. Para entrada de texto con echo (nombre del jugador via `std::getline`), se restaura ECHO temporalmente:

```cpp
Platform::echoOn();
std::getline(std::cin, nombreInput);
Platform::echoOff();
```

En Windows es no-op porque `std::getline` maneja echo nativamente.

---

## Makefile cross-platform

```makefile
ifeq ($(OS),Windows_NT)
    TARGET := leyenda.exe
    STATIC_LIBS := -static-libgcc -static-libstdc++ -static
else
    TARGET := leyenda
    STATIC_LIBS := -static-libgcc -static-libstdc++
endif
```

| Característica | Windows | Linux |
|---|---|---|
| Binario | `leyenda.exe` | `leyenda` (sin extensión) |
| Static linking | `-static` (todo incluido) | solo runtime libraries |
| Crear directorios | `if not exist "x" mkdir "x"` | `mkdir -p "x"` |
| Copiar archivos | `copy /y` | `cp` |
| Borrar directorios | `rmdir /s /q` | `rm -rf` |
| Ejecutar | `leyenda.exe` | `./leyenda` |

### Targets

| Target | Comando | Descripción |
|--------|---------|-------------|
| `all` | `make` | Build debug → `leyenda.exe` |
| `dist` | `make dist` | Optimizado (-O2), estático, copia datos a `.dist/` |
| `run` | `make run` | Build + ejecutar |
| `clean` | `make clean` | Elimina obj/, obj-dist/, ejecutable |

---

## Compilar

```bash
# Windows (MinGW/MSYS2)
make

# Linux
make

# Distribución (estático, optimizado)
make dist

# Ejecutar
make run
```

Requiere g++ con soporte C++17:
```bash
# Debian/Ubuntu
sudo apt install g++ make
```

---

## Dependencias

```
Platform.hpp  → (standalone, solo includes del SO)
main.cpp      → GameManager.hpp, Platform.hpp
Batalla.cpp   → Platform.hpp, ..., Inventario.hpp
GameManager.cpp → ..., Platform.hpp
Inventario.cpp → ..., Platform.hpp
```
