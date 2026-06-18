## Log
### Log 18/06/2026 — Soporte multiplataforma (Linux + Windows)

#### Cambios realizados

| # | Cambio | Archivos afectados |
|---|--------|--------------------|
| 1 | **`Platform.hpp`** — Nueva capa de abstracción multiplataforma. Unifica init de terminal, raw mode, terminal size y single-key input. Windows: `SetConsoleMode`/`GetConsoleScreenBufferInfo`/`_getch()`. Linux: `termios` raw mode + `ioctl(TIOCGWINSZ)` + `read()`. | `lib/Platform.hpp` (nuevo) |
| 2 | **`main.cpp`** simplificado — Eliminado `#include <windows.h>` y las llamadas directas a Win32 API. Ahora `Platform::initTerminal()` + `atexit(Platform::restoreTerminal)` maneja ambos SO. | `src/main.cpp` |
| 3 | **Input cross-platform** — Reemplazados `std::cin.get(key)` en combate, `std::cin >> input` en overworld, y `std::cin.get(key)` en inventario por `Platform::getKey()`. En Windows usa `_getch()`, en Linux usa `read()` con raw mode. Ya no requiere Enter para las teclas de juego. | `src/Batalla.cpp`, `src/GameManager.cpp`, `src/Inventario.cpp` |
| 4 | **Terminal size detect cross-platform** — `ScreenBuffer::getTerminalWidth/Height()` ya no usa directamente `GetConsoleScreenBufferInfo`, delega a `Platform::getTerminalWidth/Height()`. | `src/Batalla.cpp` |
| 5 | **`echoOn()`/`echoOff()`** — Para entrada de texto con echo (nombre del jugador al iniciar partida). En Windows no-op (std::getline maneja echo nativamente), en Linux togglea ECHO flag del termios. | `lib/Platform.hpp`, `src/GameManager.cpp` |
| 6 | **Makefile cross-platform** — Detección automática con `ifeq ($(OS),Windows_NT)`. Windows: `leyenda.exe`, `copy`, `rmdir`. Linux: `leyenda` (sin extensión), `cp`, `rm -rf`. Static linking condicional (`-static` solo en Windows). | `Makefile` |
| 7 | **`limpiarBuffer()`** eliminado donde no era necesario — Los `cin.ignore()`/`limpiarBuffer()` previos a `cin.get()` ya no aplican porque `Platform::getKey()` no tiene buffer residual. | `src/Batalla.cpp`, `src/GameManager.cpp` |
| 8 | **`.gitignore`** — Agregado `obj-dist/` para excluir artefactos de `make dist`. | `.gitignore` |

#### Por qué se hicieron

**1-5. Abstracción multiplataforma**
El juego original solo funcionaba en Windows con MinGW. Dependía de `windows.h` para activar ANSI (VT processing), detectar tamaño de terminal y leer teclas. En Linux:
- ANSI funciona nativamente (no requiere `SetConsoleMode`)
- El tamaño de terminal se obtiene con `ioctl(TIOCGWINSZ)` (no `GetConsoleScreenBufferInfo`)
- El input single-key requiere raw mode con `termios` (ICANON off, ECHO off)
- `std::cin.get()` en Linux en modo cooked requiere Enter, lo que hace imposible el juego en tiempo real

`Platform.hpp` centraliza todas estas diferencias detrás de funciones inline simples. El resto del código ya no necesita `#ifdef _WIN32` desperdigados.

**3. Raw mode en Linux**
`Platform::initTerminal()` en Linux desactiva ICANON y ECHO con `tcsetattr()`. Esto hace que `read()` (usado por `Platform::getKey()`) retorne inmediatamente al presionar una tecla, sin esperar Enter. El juego responde en tiempo real como en Windows con `_getch()`.

**5. echoOn/echoOff para entrada de nombre**
Con ECHO off globalmente, `std::getline()` para el nombre del jugador no mostraría lo que escribe. `echoOn()` restaura ECHO temporalmente para esa entrada, y `echoOff()` lo vuelve a desactivar después.

**6. Makefile unificado**
El Makefile ahora detecta automáticamente el SO. En Linux:
- El binario se llama `leyenda` (sin `.exe`)
- Usa `mkdir -p`, `cp`, `rm -f` (comandos POSIX estándar)
- No usa `-static` (los SO modernos manejan dependencias con el package manager)
- El target `dist` copia archivos con `cp`

#### Detalle técnico: Mecanismo de raw mode en Linux

```cpp
struct termios raw;
tcgetattr(STDIN_FILENO, &orig_termios);  // guardar estado original
raw = orig_termios;
raw.c_lflag &= ~(ECHO | ICANON);         // sin echo, sin buffering de línea
raw.c_cc[VMIN] = 1;                      // read() retorna con 1 byte
raw.c_cc[VTIME] = 0;                     // sin timeout
tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // aplicar
```

- `ECHO`: evita que lo que se escribe aparezca en pantalla (el juego ya dibuja su propia UI)
- `ICANON`: modo no-canónico (cada byte está disponible inmediatamente, no se espera a \n)
- `VMIN=1`: la llamada `read()` retorna tan pronto como haya al menos 1 byte disponible
- `restoreTerminal()` se llama con `atexit()` para garantizar que la terminal vuelva a su estado normal incluso si el juego crashea

#### Detalle técnico: getKey() cross-platform

```cpp
// Windows
return char(_getch());      // conio.h, no requiere Enter, sin echo

// Linux
char c;
read(STDIN_FILENO, &c, 1);  // raw mode activo, retorna inmediatamente
return c;
```

Ambos retornan el código ASCII de la tecla presionada sin mostrar el carácter en pantalla y sin requerir Enter. El menú de combate con W/S/SPACE y el overworld con WASD responden igual en ambos SO.

#### Detalle técnico: Terminal size en Linux

```cpp
struct winsize w;
if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
    return w.ws_col;
// Fallback a variable de entorno COLUMNS
// Fallback final a 80
```

`ioctl(TIOCGWINSZ)` consulta el tamaño real de la ventana de terminal. Si falla (redirección, SSH sin PTY, etc.), cae a la variable de entorno `COLUMNS`/`LINES`. Si tampoco existe, retorna 80x24 como valor seguro.

#### Compilar en Linux

```bash
# Compilar
make

# Compilar distribución (estático, optimizado)
make dist

# Ejecutar
make run

# Limpiar
make clean
```

Requiere `g++` con soporte C++17. En distribuciones basadas en Debian:
```bash
sudo apt install g++ make
```

#### Testing en Windows

El juego se compila y prueba en Windows con MinGW (MSYS2). `make all` produce `leyenda.exe` con dependencias dinámicas (necesita DLLs de MinGW). `make dist` produce un ejecutable estático en `.dist/` que solo requiere `KERNEL32.dll` y `msvcrt.dll`.

#### Archivos modified

| Archivo | Cambio |
|---------|--------|
| `lib/Platform.hpp` | Nuevo — 112 líneas, header-only |
| `src/main.cpp` | 17→12 líneas, usa Platform en vez de Win32 directo |
| `src/Batalla.cpp` | Terminal size via Platform, input via getKey, quitados limpiarBuffer |
| `src/GameManager.cpp` | Input via getKey, echo on/off para nombre, quitado cin.ignore |
| `src/Inventario.cpp` | Input loop usa getKey en vez de cin.get |
| `Makefile` | Detección de SO, comandos separados, run con ./ |
| `.gitignore` | +obj-dist/ |

### Log 18/06/2026 — Renovación total del inventario

#### Cambios realizados

| # | Cambio | Archivos afectados |
|---|--------|--------------------|
| 1 | **`InvRenderer` + `InventoryUI`** — Nuevo sistema de inventario con UI tipo combate (ScreenBuffer, ANSI, redibujado diferencial). Panel con lista scrolleable, detalle del item, stats del jugador, pestañas de categoría (Armas/Consumibles/Clave), navegación por teclado (W/S/A/D/SPACE/Q), overlay sobre la pantalla actual sin limpiar. | `lib/Inventario.hpp` (nuevo), `src/Inventario.cpp` (nuevo) |
| 2 | **`batalla.cpp`: integración del inventario en combate** — Reemplazado el viejo sistema de `std::cout`/`std::cin` + `limpiarPantalla()` por `InventoryUI`. Eliminados `suppressCout()`/`restoreCout()` alrededor del inventario (no aplica porque InventoryUI usa su propio ScreenBuffer). | `src/batalla.cpp` |
| 3 | **`batalla.cpp`: ancho del menú reducido** — `menuW` cambiado de 26 a 22 para evitar solapamiento con el panel de información del jugador (columna 28 compartida causaba corrimiento visual al navegar). | `src/batalla.cpp` |
| 4 | **`GameManager.cpp`: integración del inventario en overworld** — Reemplazado `Jugador::mostrarInventario()` por `InventoryUI`. Agregado `std::cin.ignore()` antes de abrir el inventario para consumir el `\n` residual de `std::cin >>`. | `src/GameManager.cpp` |
| 5 | **`Enemigo.cpp`: constructor faltaba `exp_base`** — El header declaraba 9 parámetros pero la implementación solo tenía 8 (faltaba `int exp_base`). Causaba error de compilación "ninguna instancia de función sobrecargada coincide". | `src/Enemigo.cpp` |
| 6 | **`Inventario.hpp`: correcciones del header** — Constructor ahora inicializa `renderer(screenBuffer)`. `setPlayerInfo()` corregido: `equippedWeaponName = equippedWeaponName` (auto-asignación) → `eqWeaName`. Agregados campos `playerExp`, `playerMaxExp`, `playerAtk`, `playerDef`. Agregado getter `getItems()`. `setLong` renombrado a `setLog`. | `lib/Inventario.hpp` |
| 7 | **Mensaje "Sin objetos" en categorías vacías** — Cuando una categoría no tiene items, `drawItemList()` muestra "(Sin objetos)" centrado en el panel en vez de dejar el box vacío. | `src/Inventario.cpp` |
| 8 | **Scroll indicators en margen derecho** — `▲`/`▼` movidos a la última columna de contenido (`listX + listW - 2`) y dibujados **después** de los items para evitar solapamiento. | `src/Inventario.cpp` |
| 9 | **Corregido orden de parámetros en `setPlayerInfo()`** — La llamada en `InventoryUI::render()` intercambiaba `getAtaque()`/`getDefensa()` con `getExperiencia()`/`getExperienciaNecesaria()`. Las stats aparecían mezcladas en el panel. | `src/Inventario.cpp` |

#### Por qué se hicieron

**1. InventoryUI**
El inventario antiguo usaba `std::cout`/`std::cin` directo con entrada de texto por nombre ("Escribe el nombre del objeto"), limpiaba toda la pantalla y no tenía ningún diseño visual. No seguía el patrón del sistema de combate (ScreenBuffer, navegación W/S, colores ANSI). El nuevo sistema:
- Reutiliza `ScreenBuffer` (doble buffer 56x22, redibujado diferencial, ANSI)
- Navegación por teclado (W/S items, A/D categorías, SPACE acción, Q salir)
- Overlay: no limpia pantalla, dibuja sobre el frame actual y al salir se restaura
- Muestra stats del jugador en todo momento (HP, MP, ATK, DEF, NIV, EXP, arma)
- Scroll para inventarios grandes con indicadores visuales
- Acciones contextuales según tipo de item (equipar arma, usar poción, ver info)

**2-4. Integración**
El viejo inventario en combate usaba `limpiarPantalla()` + `std::cout`, rompiendo el frame de batalla. El nuevo `InventoryUI` es un overlay: escribe sobre el buffer de combate y al salir se restaura con `forceRedraw()`. En el overworld, el flujo es igual: `limpiarPantalla()` + `renderMapa()` después de cerrar el inventario.

**5. Constructor de Enemigo**
`EnemyFactory` llamaba al constructor con `exp_base` pero la implementación no lo aceptaba, error de compilación.

**6. Bugfixes del header**
El constructor no inicializaba `renderer` (no tiene default constructor). Auto-asignación en `setPlayerInfo` ignoraba el parámetro `eqWeaName`.

**7-8. UX de la lista**
Categorías vacías ahora muestran feedback visual. Scroll indicators ya no se pisaban con los items.

**9. Stats mezcladas**
`getAtaque()` iba a `pExp` y `getExperiencia()` a `pAtk`, mostrando números en campos incorrectos.

#### Layout del inventario

```
L0:  === LEYENDA DEL CAMPUS - INVENTARIO ===   (BYELLOW)
L1:  ────────────────────────────────────────   (CYAN)
L2:   [Armas] [Consumibles] [Clave]            (tabs, seleccionada en BYELLOW)
L3:  ┌──────────────────┬────────────────────┐
L4:  │ > Espada Leyenda  │  Daño: +20         │
L5:  │   Daga Plateada   │  "Poderosa espada" │
L6:  │   Bastón Arcano   │                     │
L7:  │   Hacha de Guerra │  Equipada: Daga    │
L8:  │   Espada Gallo    │  (+8)              │
L9:  │                ▲ │                     │
L10: │                ▼ │                     │
L11: └──────────────────┴────────────────────┘
L12: ┌─ HEROE ──────────────────────────────────┐
L13: │ HP ▓▓▓▓▓▓▓▓▓░░ 30/40  MP ▓▓▓▓▓▓░░ 20/30│
L14: │ ATK: 15  DEF: 10  NIV: 3                │
L15: │ EXP: 150/300  ARMA: Daga (+8)           │
L16: └───────────────────────────────────────────┘
L17:
L18:
L19:
L20:
L21: [W/S] Navegar [A/D] Categoria [SPACE] OK [Q] Salir
```

#### Clases nuevas

```
InvRenderer — Renderiza cada sección del inventario sobre un ScreenBuffer
  - drawBackground()         → título + separador
  - drawCategoryTabs()       → pestañas Armas/Consumibles/Clave
  - drawItemList()           → lista scrolleable con scroll indicators
  - drawItemDetails()        → panel derecho con info del item seleccionado
  - drawPlayerStats()        → panel inferior con HP, MP, ATK, DEF, NIV, EXP, arma
  - drawFooter()             → controles en la última línea
  - renderAll()              → compone todo y llama a buf.render()

InventoryUI — Orquestador del inventario (máquina de estados)
  - buildItemList(cat)       → filtra objetos del jugador por tipo
  - processInput(key)        → maneja W/S/A/D/SPACE/Q según el estado
  - doAction()               → equipa arma, usa poción, o muestra descripción
  - render()                 → pasa datos del jugador al renderer y dibuja
  - run()                    → bucle principal (render + input hasta CLOSED)
```

#### Estados del inventario

```cpp
enum class InvState {
    BROWSING,        // navegando la lista de items
    ITEM_ACTIONS,    // submenú de acciones (Usar/Equipar/Info)
    CONFIRM_ACTION,  // confirmación "¿Usar X?"
    CLOSED           // salir del inventario
};
```

#### Controles del inventario

| Tecla | Acción |
|---|---|
| **W/S** | Navegar items (circular, arriba/abajo) |
| **A/D** | Cambiar categoría (Armas → Consumibles → Clave) |
| **SPACE** | Ejecutar acción sobre item seleccionado |
| **Q** | Cerrar inventario |

#### Integración desde combate

```cpp
// En batalla.cpp, case 2 (Inventario):
InventoryUI invUI(*player);
invUI.run();
screenBuffer.forceRedraw();  // restaurar frame de combate
currentState = BattleState::PLAYER_TURN;
```

No se usa `suppressCout()` porque `InventoryUI` tiene su propio `ScreenBuffer` y escribe directamente a `std::cout` para los ANSI. Solo se silencia si la acción del item (ej. `usarPocion`) llamara a `std::cout`, pero en `doAction()` no hay suppress — los mensajes se capturan en `logMessage` y se muestran en el propio inventario.

#### Integración desde overworld

```cpp
// En GameManager::mostrarInventario():
InventoryUI invUI(jugador);
invUI.run();
limpiarPantalla();
renderMapa();
```

Es necesario `std::cin.ignore()` antes de abrir el inventario porque `std::cin >>` en el loop OVERWORLD deja un `\n` residual.

---

### Explicación detallada de funciones y clases

#### `InvRenderer` — Renderizador del inventario

```
InvRenderer(ScreenBuffer &buffer) : buf(buffer) {}
```

Recibe una referencia a un `ScreenBuffer` (el mismo que usa combate, pero `InventoryUI` crea el suyo propio independiente). Dibuja 7 secciones:

| Función | Líneas | Contenido |
|---|---|---|
| `drawBackground()` | 0-1 | Título "=== LEYENDA DEL CAMPUS - INVENTARIO ===" en `COL_BYELLOW`, separador de guiones en `COL_CYAN` |
| `drawCategoryTabs()` | 2 | Tres pestañas: `[Armas] [Consumibles] [Clave]`. La pestaña activa se pinta en `COL_BYELLOW`, las inactivas en `COL_CYAN`. Posición calculada dinámicamente según el ancho de cada string |
| `drawItemList()` | 4-11 | Box de 26x9. Calcula `visibleStart` para scroll (si `selectedIndex` sale del rango visible, desplaza el inicio). Itera items visibles, pinta ` > nombre xN` para el seleccionado y `   nombre xN` para los no seleccionados. Trunca a `listW - 3 = 23` caracteres reemplazando con `..`. Draws scroll indicators `▲`/`▼` en la última columna de contenido (`listX + listW - 2`) **después** de los items para evitar solapamiento |
| `drawItemDetails()` | 4-11 (panel derecho) | Box de 25x9. Mediante `dynamic_pointer_cast` detecta el tipo real del objeto: `Arma` → muestra "Daño: +X" en amarillo, `Pocion` → muestra "Cura: +X HP" en verde. Descripción con word-wrap manual (3 líneas de `detW - 4 = 21` caracteres cada una). Si `selectedIndex` está fuera de rango o la lista vacía, no hace nada (early return) |
| `drawPlayerStats()` | 14-16 | Box de 52x5. Tres líneas: (1) Nombre + HP bar (10 chars, color según %: verde >50%, amarillo >25%, rojo) + numérico HP, MP bar (10 chars, azul) + numérico MP. (2) "ATK: X DEF: X NIV: X" en blanco. (3) "EXP: X/Y" en amarillo + "ARMA: nombre (+X)" en amarillo si hay arma equipada |
| `drawFooter()` | 21 | "[W/S] Navegar [A/D] Categoria [SPACE] OK [Q] Salir" centrado en `COL_CYAN` |
| `renderAll()` | — | `buf.clear()`, llama a las 6 funciones anteriores en orden, y finalmente `buf.render()` que vuelca a terminal con redibujado diferencial |

#### `InventoryUI` — Orquestador del inventario

```
InventoryUI(Jugador &p) : player(&p), renderer(screenBuffer) {}
```

Crea su propio `ScreenBuffer` (independiente del de combate). `run()` es el bucle principal:

```
run()
  ├─ hideCursor()
  ├─ state = BROWSING, currentCategory = ARMAS, selectedIndex = 0
  ├─ buildItemList(ARMAS) → carga items iniciales
  ├─ while(state != CLOSED):
  │     render()              → dibuja frame completo
  │     cin.get(key)          → espera tecla
  │     processInput(key)     → navega o ejecuta acción
  └─ showCursor()
```

**`buildItemList(ItemCategory cat)`**:
Recorre `jugador->getObjetosInventario()` (un `map<string, shared_ptr<Objeto>>`). Para cada par, compara `objeto->getTipo()` contra la categoría actual:
- `"Arma"` → `ARMAS`
- `"Pocion"` → `POCIONES`
- `"Objeto Clave"` → `CLAVE`

Si coincide, busca la cantidad en `jugador->getInventario()` (el `map<string, int>` paralelo) y construye un `ItemEntry{nombre, objeto, cantidad}`. Retorna `vector<ItemEntry>`.

**`processInput(char key)`**:
Máquina de estados interna:

- **`BROWSING`**:
  - `q/Q` → `state = CLOSED` **(se chequea antes que items.empty() para poder salir aunque no haya objetos)**
  - `w/W` → navega hacia arriba (circular: si está en 0, va al último). Si `items` vacío, no hace nada
  - `s/S` → navega hacia abajo (circular). Si items vacío, no hace nada
  - `a/A` → `currentCategory = (currentCategory - 1 + 3) % 3`, reconstruye lista con `buildItemList()`, reset `selectedIndex = 0`
  - `d/D` → `currentCategory = (currentCategory + 1) % 3`, igual que A
  - `SPACE` → si `!items.empty()`, `state = ITEM_ACTIONS`
- **`ITEM_ACTIONS`**:
  - `SPACE` → `doAction()`, vuelve a `BROWSING`
  - `q/Q/ESC` → vuelve a `BROWSING`
- **`CONFIRM_ACTION`**: (reservado para futura confirmación de uso/descarte)
- **`default`**: `break` (silencia warning de switch incompleto)

**`doAction()`**:
Obtiene `items[selectedIndex]` y usa `dynamic_pointer_cast` para determinar el tipo:
- `Arma` → `jugador->equiparArma(arma)` (hereda de `Objeto` que no es polimórfico en el parámetro `equiparArma`, pero el método acepta `shared_ptr<Arma>` directamente). Setea `logMessage = "Has equipado: " + nombre`
- `Pocion` → `jugador->usarPocion(pocion.get())` (pasa raw pointer obtenido con `.get()`) + `jugador->eliminarObjeto(nombre)` (decrementa cantidad, si llega a 0 borra la entrada del inventario). Reconstruye la lista visible con `buildItemList(currentCategory)` porque el inventario cambió
- Otro (ObjClave, etc.) → `logMessage = obj->getDescripcion()`

**`render()`**:
Pasa los 12 campos del jugador al renderer:
```
setPlayerInfo(
    salud, saludMax, mana, manaMax, nombre, nivel,
    exp, expMax, ataque, defensa, armaNombre, armaDmg
)
```
Luego `setLogMessage(logMessage)` y `renderer.renderAll()`.

#### Integración con el sistema de combate

**En `BattleSystem::doPlayerAction()`, case 2: Inventario**:

```cpp
InventoryUI invUI(*player);
invUI.run();
screenBuffer.forceRedraw();
currentState = BattleState::PLAYER_TURN;
```

El flujo completo:

```
Combate (PLAYER_TURN)
  → Jugador presiona SPACE en "Inventario" (opción 2)
  → currentState = PLAYER_ACTION
  → doPlayerAction() case 2:
      1. Se crea InventoryUI con el *player actual
      2. InventoryUI::run() toma control total de la terminal
         - Oculta el cursor
         - Bucle: renderiza frame del inventario → espera tecla → procesa
         - El ScreenBuffer del inventario escribe sobre las mismas líneas
           que ocupaba el frame de combate (líneas 0-21)
         - El frame de combate NO se restaura durante el inventario
         - Al presionar Q, state = CLOSED, sale del bucle
      3. screenBuffer.forceRedraw() invalida el caché del buffer de combate
      4. currentState = PLAYER_TURN (vuelve al bucle principal de combate)
      5. En la siguiente iteración, BattleSystem::render() redibuja
         el frame de combate completo (forceRedraw causó firstFrame=true)
```

**Por qué NO se usa `suppressCout()` aquí:**
- `InventoryUI` usa su propio `ScreenBuffer` que escribe ANSI directamente a `std::cout` mediante `ScreenBuffer::render()`. Si `suppressCout()` estuviera activo, esos ANSI irían al `ostringstream` y el usuario **no vería el inventario** (vería el frame de combate congelado).
- Dentro de `doAction()`, `jugador->usarPocion()` y `jugador->equiparArma()` tienen `std::cout` internos, pero como el inventario se renderiza inmediatamente después, esos mensajes se pierden en la terminal sin afectar el buffer. No causan desincronización porque el `ScreenBuffer` del inventario sobrescribe completamente las líneas relevantes en cada frame.

**Diferencia con el inventario antiguo en combate:**
- Antes: `limpiarPantalla()` + `jugador->mostrarEstado()` + `cout << "Inventario"` + `getline(cin, nombre)` → rompía el frame de combate por completo
- Ahora: overlay en el mismo buffer → `forceRedraw()` restaura el frame → transición invisible

#### Integración con el overworld

**En `GameManager::mostrarInventario()`**:

```cpp
InventoryUI invUI(jugador);
invUI.run();
limpiarPantalla();
renderMapa();
```

El flujo:

```
Overworld (loop de exploración)
  → Jugador presiona 'I'
  → GameManager::mostrarInventario():
      1. Se crea InventoryUI con el jugador
      2. InventoryUI::run() toma control
         - Escribe sobre las líneas 0-21 usando su ScreenBuffer
         - El mapa renderizado previamente con std::cout queda debajo
           (el ScreenBuffer solo reescribe las líneas que cambian)
      3. Al salir, se llama limpiarPantalla() + renderMapa()
         porque el mapa no usa ScreenBuffer — es std::cout directo.
         No hay un "frame anterior" que restaurar como en combate.
```

**Problema del `\n` residual:**
- El loop OVERWORLD usa `std::cin >> input;` que consume un carácter pero deja el `\n` en el buffer
- `InventoryUI::run()` usa `std::cin.get(key)` que lee un solo carácter
- Si no se ignora el `\n` antes, el primer `cin.get()` del inventario se traga el `\n`, y el usuario tiene que presionar DOS teclas para la primera acción
- Solución: `std::cin.ignore(numeric_limits<streamsize>::max(), '\n')` justo antes de abrir el inventario

#### Diferencia clave: overlay en combate vs overworld

| Aspecto | En combate | En overworld |
|---|---|---|
| Qué hay debajo | Frame de combate (ScreenBuffer 56x22) | Mapa renderizado con `std::cout` directo |
| Cómo se restaura | `screenBuffer.forceRedraw()` → BattleSystem::render() redibuja todo | `limpiarPantalla()` + `renderMapa()` desde cero |
| ScreenBuffer del inventario | Crea su propio buffer independiente, escribe sobre la terminal en las mismas coordenadas | Ídem |
| Sin limpiar pantalla | Sí — es overlay real, el frame de combate se restaura sin borrar | No del todo — se requiere `limpiarPantalla()` al salir porque el mapa no tiene buffer propio |
| Input fantasma | No hay `\n` residual (combate usa `cin.get()` consistentemente) | Sí, requiere `cin.ignore()` |

### Log 15/06/2026 (sesión 3) — Sistema de guardado con caché

#### Cambios realizados

| # | Cambio | Archivos afectados |
|---|--------|--------------------|
| 1 | **CacheManager con estado completo** — `guardarHeroe()` ahora persiste: posición (posX, posY), saludMax, manaMax, exp, expMax, arma equipada e inventario completo (array de `{nombre, cant}`). `cargarHeroe()` recibe el catálogo de objetos para resolver nombres del inventario. | `lib/CacheManager.hpp`, `src/CacheManager.cpp` |
| 2 | **`agregarObjetoSilencioso()`** — Nuevo método en Jugador que agrega objetos al inventario sin prompt de equipar (necesario para cargar partida desde caché sin interacción del usuario). | `lib/Jugador.hpp`, `src/Jugador.cpp` |
| 3 | **`batalla()` guarda en caché** — Las llamadas a `DataManager::guardarHeroe()` (que escribía sobre `json/heroe.json`) se reemplazan por `CacheManager::guardarHeroe()` (escribe en `cache/heroe.json`). El archivo original `json/heroe.json` ya no se modifica. | `src/Batalla.cpp` |
| 4 | **GameManager: menú Nueva Partida / Continuar / Salir** — `mostrarMenuPrincipal()` ahora muestra 3 opciones. "Nueva Partida" limpia `cache/`, carga mapa original + héroe default. "Continuar" carga mapa y héroe desde `cache/`. "Salir" termina el juego. El constructor ya no carga héroe desde `json/heroe.json`. | `lib/GameManager.hpp`, `src/GameManager.cpp` |
| 5 | **Persistencia en tiles del mapa** — `handleTile()` ahora llama a `CacheManager::guardarMapa()` después de modificar tiles 'B' (jefe derrotado → '.') y 'H' (poción recogida → '.'). | `src/GameManager.cpp` |
| 6 | **Guardado al salir con Q** — La tecla 'Q' en OVERWORLD ahora llama a `guardarPartida()` que persiste héroe + mapa antes de salir. | `src/GameManager.cpp` |
| 7 | **Bugfix `esTransitable`** — Corregido `'p'` (minúscula) → `'P'` (mayúscula) para que el tile de spawn sea transitable. | `src/Mapa.cpp` |
| 8 | **Variables sin uso removidas** — Eliminadas `saludMaxima` y `manaMaxima` en `CacheManager::cargarHeroe()` que se leían del JSON pero nunca se usaban. | `src/CacheManager.cpp` |
| 9 | **Tile 'P' se elimina al moverse** — Al salir de la casilla de spawn 'P', se reemplaza por '.' y se persiste en caché, para que no quede visible en el mapa tras moverse. | `src/GameManager.cpp` |

#### Por qué se hicieron

**1-2. Estado completo del héroe**
Antes `CacheManager::guardarHeroe()` solo guardaba 6 campos básicos (nombre, salud, ataque, defensa, nivel, pociones). Al cargar una partida, el jugador perdía: posición en el mapa, inventario de objetos, arma equipada, experiencia acumulada y maná. Ahora se guardan 14 campos incluyendo el inventario como array JSON y el nombre del arma. `cargarHeroe()` reconstruye el jugador usando el catálogo de objetos para resolver nombres a `shared_ptr<Objeto>`.

**3. batalla() escribe en caché**
Antes `batalla()` llamaba a `DataManager::guardarHeroe()` que escribía en `json/heroe.json`, contaminando los archivos originales. Ahora escribe en `cache/heroe.json`, dejando los JSONs originales intactos para futuras partidas nuevas.

**4-6. Flujo completo de partidas**
Antes no existía distinción entre "nueva partida" y "continuar". El juego siempre cargaba desde `json/heroe.json`, haciéndolo incompatible con múltiples partidas o con un sistema de guardado persistente. Ahora:
- `cache/` almacena exclusivamente el estado de la partida en curso
- `json/` y `mapas/` son solo lectura (orígenes de datos)
- El menú permite elegir entre empezar de cero o retomar

**7. Bugfix**
`esTransitable()` tenía `'p'` en minúscula pero el mapa usa `'P'` mayúscula para el spawn. Esto causaba que la casilla de inicio no fuera transitable después de moverse, aunque el bug no se manifestaba porque el jugador nunca necesitaba volver a pisar la 'P' original.

**9. Tile 'P' se limpia al salir del spawn**
Antes el tile 'P' permanecía visible en el mapa después de que el jugador se moviera, mostrando una 'P' huérfana en la posición inicial. Ahora `moverJugador()` captura la posición vieja antes de moverse y, si contenía 'P', la reemplaza por '.' y persiste el cambio en `cache/mapa_cache.txt`.

#### Flujo de guardado actual

```
INICIO → Menú (Nueva Partida / Continuar / Salir)
          │
          ├─ "Nueva Partida"
          │   ├─ CacheManager::limpiar()           → borra cache/
          │   ├─ mapa.cargar(original)              → mapa fresco
          │   ├─ Jugador("Heroe") + equipar armas   → héroe default
          │   └─ CacheManager::crearPartida()       → crea cache/ con estado inicial
          │
          ├─ "Continuar"
          │   ├─ CacheManager::existePartida()?     → busca cache/partida.flag
          │   ├─ CacheManager::cargarMapa()         → mapa con tiles modificados
          │   └─ CacheManager::cargarHeroe(objetos)  → héroe con inventario y posición
          │
          └─ "Salir" → return

DURANTE EL JUEGO
  ├─ Salir de spawn 'P' → mapa.setTile('.') → CacheManager::guardarMapa()
  ├─ Pisar tile 'B' (jefe) → batalla() → mapa.setTile('.') → CacheManager::guardarMapa()
  ├─ Pisar tile 'H' (poción) → usarPocion() → mapa.setTile('.') → CacheManager::guardarMapa()
  ├─ Combate aleatorio → batalla() → CacheManager::guardarHeroe()  (dentro de batalla)
  └─ Tecla 'Q' → guardarPartida() → CacheManager::guardarHeroe() + guardarMapa() → salir
```

#### Formato del archivo `cache/heroe.json`
```json
{
    "nombre": "Heroe",
    "salud": 85,
    "saludMaxima": 100,
    "ataque": 25,
    "defensa": 15,
    "nivel": 2,
    "pociones": 2,
    "mana": 40,
    "manaMaxima": 60,
    "posX": 5,
    "posY": 7,
    "exp": 120,
    "expMax": 300,
    "arma": "Espada Gallo",
    "inventario": [
        { "nombre": "Pocion Milagrosa", "cant": 2 },
        { "nombre": "Adblock", "cant": 1 }
    ]
}
```

#### Dependencias actualizadas
```
CacheManager → Config, json.hpp, Mapa.hpp, Jugador.hpp, Objeto.hpp
GameManager  → DataManager, CacheManager, Batalla, Config, ArtLoader
Batalla      → ... CacheManager  (nuevo include)
```

### Log 15/06/2026 (sesión 2) — Separación de Managers

#### Cambios realizados

| # | Cambio | Archivos afectados |
|---|--------|--------------------|
| 1 | **DataManager** — Nuevo namespace que centraliza toda la carga/guarda de datos JSON (objetos, enemigos, héroe). Reemplaza las funciones libres `cargarObjetosDesdeJSON`, `cargarEnemigosDesdeJSON`, `cargarHeroe`, `guardarHeroe`, `generarEnemigoPorNivel` que estaban en `batalla.hpp/.cpp`. Usa `Config` para todos los paths. | `lib/DataManager.hpp` (nuevo), `src/DataManager.cpp` (nuevo) |
| 2 | **GameManager** — Nueva clase que orquesta el loop principal del juego. Encapsula: carga de datos, renderizado del mapa, movimiento WASD, eventos de tiles (E/H/K/B), inicio de combate, inventario, condición de victoria. `main.cpp` se reduce a 4 líneas. | `lib/GameManager.hpp` (nuevo), `src/GameManager.cpp` (nuevo) |
| 3 | **batalla.hpp/.cpp limpiado** — Eliminadas todas las funciones de carga de datos (`cargarObjetosDesdeJSON`, `cargarEnemigosDesdeJSON`, `cargarHeroe`, `guardarHeroe`, `generarEnemigoPorNivel`, `dataPath()`, `rng()`). `batalla()` ahora acepta `Enemigo&` directamente en vez del mapa de enemigos. Eliminado `#include "json.hpp"`. | `lib/batalla.hpp`, `src/batalla.cpp` |
| 4 | **CacheManager desacoplado** — Ya no depende de `batalla.hpp` para heroe IO. Implementa su propia serialización JSON contra `Config::heroeCachePath()`. No incluye ningún header de combate. | `lib/cacheManager.hpp`, `src/cacheManager.cpp` |
| 5 | **Dead code eliminado** — Removidos `cargadorArchivos.hpp`/`.cpp` (nunca usado) y `Jugador::mostrarMenu()` (menú de combate antiguo, no llamado por nada). | `lib/cargadorArchivos.hpp` (eliminado), `src/cargadorArchivos.cpp` (eliminado), `lib/jugador.hpp`, `src/jugador.cpp` |
| 6 | **Paths unificados** — `Config::heroePath()` ahora apunta a `json/heroe.json` (antes era `cache/heroe.json`). Agregado `Config::heroeCachePath()` para el path de cache separado. | `lib/config.hpp` |
| 7 | **Makefile** — Target renombrado de `batalla.exe` a `leyenda.exe`. | `Makefile` |

#### Por qué se hicieron

**1. DataManager**
Antes las funciones de carga JSON estaban mezcladas en `batalla.hpp/.cpp` junto con la lógica de combate (ScreenBuffer, Renderer, BattleSystem). Cualquier archivo que necesitara cargar datos (como `cacheManager.cpp`) tenía que incluir toda la maquinaria de batalla. Ahora `DataManager` es un namespace independiente con una responsabilidad única: lectura/escritura de datos desde JSON.

**2. GameManager**
`main.cpp` tenía 131 líneas con el loop principal, paths hardcodeados, carga de datos, renderizado y lógica de movimiento todo mezclado. Ahora `GameManager` encapsula el ORQUESTADOR del juego, liberando a `main.cpp` para que sea solo el punto de entrada. Cada responsabilidad (render, movimiento, tiles) es un método separado.

**3. batalla.hpp/.cpp limpiado**
El archivo pasó de ~869 líneas (con data loading) a ~728 líneas (solo combate). La separación reduce el acoplamiento: un cambio en el formato JSON ya no requiere tocar el código de combate, y viceversa. La nueva firma `batalla(Jugador&, Enemigo&)` es más simple — el caller ya eligió el enemigo.

**4. CacheManager desacoplado**
Antes incluía `batalla.hpp` solo para llamar a `::guardarHeroe()` y `::cargarHeroe()`, arrastrando toda la cadena de includes (json.hpp, objeto.hpp, enemigo.hpp, jugador.hpp, combate). Ahora es autónomo: serializa JSON directamente y solo depende de `config.hpp`.

**5. Dead code**
`cargadorArchivos.hpp/.cpp` era un struct `Cargador` con `archivoExiste()` y `cargarJSON()` que nunca se llamaba desde ningún archivo. `Jugador::mostrarMenu()` era el menú de combate del sistema anterior (1. Atacar, 2. Magia, 3. Usar pocion...) reemplazado por `BattleSystem` + `InputHandler`.

#### Dependencias finales

```
DataManager → Config, json.hpp, objeto.hpp, enemigo.hpp, jugador.hpp
GameManager → DataManager, batalla.hpp, config.hpp, mapa.hpp, jugador.hpp
CacheManager → Config, json.hpp
batalla.hpp → enemigo.hpp, jugador.hpp  (ya NO incluye json.hpp)
main.cpp → GameManager.hpp  (4 líneas)
```

`CacheManager` y `batalla.hpp` ya no tienen dependencia mutua.

#### Diagrama de archivos actual

```
src/
├── main.cpp              # 4 líneas: crea GameManager y corre
├── GameManager.cpp       # Loop principal, movimiento, eventos
├── DataManager.cpp       # Carga/guarda JSON (objetos, enemigos, héroe)
├── batalla.cpp           # Solo combate (ScreenBuffer, Renderer, BattleSystem)
├── cacheManager.cpp      # Solo cache de partida (independiente)
├── config.cpp            # Paths de datos
├── jugador.cpp           # Jugador, inventario, nivelación
├── Enemigo.cpp           # Enemigo, loot
├── mapa.cpp              # Mapa 2D, colisiones
└── (cargadorArchivos.cpp eliminado)
```

### Log 15/06/2026 — Enemy Factory, Encuentros Aleatorios, Máquina de Estados

#### Cambios realizados

| # | Cambio | Archivos afectados |
|---|--------|--------------------|
| 1 | **Nuevo formato JSON de enemigos** — Cambiado de array plano a objeto con `"enemigos"` que agrupa por nivel. Cada enemigo tiene `id`, `peso`, `ascii` (array de 6 líneas), `botin` (array de objetos con `nombre` + `probabilidad`), `boss` (bool). Eliminados campos viejos `loot1`/`loot2`/`prob1`/`prob2`. | `json/enemigos.json` |
| 2 | **Enemigo extendido** — Nuevos miembros: `id`, `asciiArt[6]`, `vector<Drop> botin`. Eliminados `loot1`/`loot2`/`prob1`/`prob2`. `Drop` cambia de `shared_ptr<Objeto>` + `int probabilidad`. | `lib/Enemigo.hpp`, `src/Enemigo.cpp` |
| 3 | **EnemyFactory** — Nueva clase que carga `json/enemigos.json`, almacena `EnemyTemplate` por nivel en `map<int, vector<EnemyTemplate>>`. Métodos: `crearEnemigo(nivel)` y `crearJefe(nivel)`. Selección ponderada por ruleta (`peso`). Búsqueda de jefe escanea desde el nivel actual hacia abajo. | `lib/EnemyFactory.hpp` (nuevo), `src/EnemyFactory.cpp` (nuevo) |
| 4 | **EncounterManager** — Nueva clase maneja encuentros aleatorios. 4 terrenos: `BOSQUE` (8%), `CAVERNA` (10%), `MAZMORRA` (15%), `ABISMO` (18%). Grace period de 3 pasos sin encuentros, +3% por paso extra, cap 40%. Contador y terreno configurables. | `lib/EncounterManager.hpp` (nuevo), `src/EncounterManager.cpp` (nuevo) |
| 5 | **GameManager con FSM explícita** — `GameState` enum (`MAIN_MENU`, `OVERWORLD`, `GAME_OVER`). Integra `EnemyFactory` y `EncounterManager`. Check aleatorio en cada paso después de eventos de tile (H/K/E/B skip). Tile `'B'` dispara jefe. Battle es sub-estado sincrónico de OVERWORLD. | `lib/GameManager.hpp`, `src/GameManager.cpp` |
| 6 | **Batalla usa datos nuevos** — `asciiArt` se copia desde `enemigo.getAsciiArt()` (6 líneas). Loot itera `vector<Drop>` con probabilidad acumulada. Eliminada `generateEnemyArt()`. | `lib/Batalla.hpp`, `src/batalla.cpp` |
| 7 | **DataManager limpiado** — Eliminados `cargarEnemigos()` y `generarEnemigoPorNivel()`. Conserva `rng()`, `cargarObjetos()`, `cargarHeroe()`, `guardarHeroe()`. | `lib/DataManager.hpp`, `src/DataManager.cpp` |
| 8 | **Tile jefe en mapa** — `'B'` agregado a `nivel1.txt` en (10,7). `esTransitable()` acepta `'B'`. | `mapas/nivel1.txt`, `src/Mapa.cpp` |
| 9 | **Documentación enemigos** — Nuevo documento que cubre: arquitectura, EnemyFactory (carga, selección ponderada, creación), EncounterManager (terrenos, fórmula, integración), formato JSON, máquina de estados, flujo completo de encuentro aleatorio y de jefe. | `docs/enemigos.md` (nuevo) |
| 10 | **Comentarios Doxygen** — Documentación estilo Doxygen en español agregada a todos los headers y sources nuevos/modificados. | `lib/EnemyFactory.hpp`, `src/EnemyFactory.cpp`, `lib/EncounterManager.hpp`, `src/EncounterManager.cpp`, `lib/Enemigo.hpp`, `src/Enemigo.cpp`, `lib/GameManager.hpp`, `src/GameManager.cpp` |

#### Por qué se hicieron

**1-3. Enemy Factory**
El sistema viejo hardcodeaba la generación de enemigos en `DataManager::generarEnemigoPorNivel()` con un `switch` estático. Cada nuevo enemigo requería recompilar. La factoría es data-driven: los enemigos se definen en JSON, se cargan en tiempo de ejecución, y la selección ponderada por `peso` permite balancear frecuencias sin tocar código. La búsqueda de jefe hacia abajo permite tener jefes solo en niveles específicos sin eliminar enemigos de niveles superiores.

**4. Encounter Manager**
Antes no existían encuentros aleatorios — el jugador pisaba `'E'` y siempre había batalla. Ahora cada paso tiene una probabilidad que: (a) depende del terreno del mapa, (b) tiene un período de gracia para evitar encuentros inmediatos al cambiar de piso, (c) aumenta progresivamente para evitar sequías largas, (d) tiene un tope para no ser abrumador.

**5. FSM explícita**
`main.cpp` tenía un loop plano con banderas sueltas. `GameManager` ahora centraliza el flujo con una máquina de estados: menú principal → exploración → (batalla anidada) → game over. Esto facilita agregar estados nuevos (INVENTARIO, TIENDA, etc.).

**6. Batalla actualizada**
El nuevo `asciiArt` de 6 líneas desde JSON reemplaza `generateEnemyArt()` que generaba monstruos de forma procedural pero limitada. El loot ahora usa probabilidades específicas por enemigo en vez de valores fijos 70/30.

#### Detalle técnico: Selección ponderada (ruleta)

El algoritmo suma todos los `peso` del nivel y genera un entero aleatorio en `[0, totalPeso)`. Luego itera los enemigos restando su peso hasta que la tirada llega a 0:

```
Rango [0, totalPeso) con pesos: Goblin=10, Orco=8, Slime=7, Murciélago=9, Zombie=6

     Goblin(10)   Orco(8)  Slime(7)  Murciélago(9)  Zombie(6)
   ┌──────────┬────────┬─────────┬──────────────┬─────────┐
   0          10       18        25             34        39

   Roll 0-9  → Goblin      (peso 10)
   Roll 10-17 → Orco        (peso 8)
   Roll 18-24 → Slime       (peso 7)
   Roll 25-33 → Murciélago (peso 9)
   Roll 34-39 → Zombie     (peso 6)
```

Cada enemigo ocupa exactamente `peso` valores en el rango. La tirada acotada garantiza que todos los segmentos son alcanzables.

#### Detalle técnico: Fórmula de encuentro

```
probabilidad = baseTerreno + max(0, pasosSinEncuentro - GRACE_PERIOD) * INCREMENTO_POR_PASO
probabilidad = min(probabilidad, CAP)
```

Con valores: GRACE_PERIOD=3, INCREMENTO=3%, CAP=40%:
- Pasos 1-3: probabilidad = baseTerreno (sin encuentros)
- Paso 4: +3%
- Paso 5: +6%
- ...hasta máximo 40%

El contador se resetea al ocurrir un encuentro. Terrenos: BOSQUE=8%, CAVERNA=10%, MAZMORRA=15%, ABISMO=18%.

#### Detalle técnico: Búsqueda de jefe

`crearJefe(nivel)` busca desde `nivel` hacia abajo hasta 1:
```cpp
for (int l = nivel; l >= 1; --l) {
    for (const auto& tplantilla : nivelTemplates[l]) {
        if (tplantilla.esJefe) return crearDesdeTemplate(tplantilla);
    }
}
```

Si no encuentra jefe en ningún nivel, lanza `std::runtime_error`.

#### Flujo de encuentro aleatorio

```
OVERWORLD
  │
  ├─ WASD ──────────→ mover jugador
  │                     │
  │                     ├─ tile 'B' ──→ crearJefe(nivel) → batalla()
  │                     ├─ tile 'K','E','H' → evento directo (sin random)
  │                     └─ tile '.' ──→ EncounterManager::checkEncounter()
  │                                       │
  │                                       ├─ true  → crearEnemigo(nivel) → batalla()
  │                                       │           │
  │                                       │           └─ victoria → OVERWORLD
  │                                       │           └─ muerte  → GAME_OVER
  │                                       │
  │                                       └─ false → OVERWORLD (sigue)
  │
  └─ 'Q' ──→ guardar → salir
```

#### Estrategia

El sistema de enemigos pasó de estático con switch a 100% data-driven. Agregar un enemigo nuevo es solo añadirlo a `json/enemigos.json` con su `peso`, `ascii`, `botin` y nivel correspondiente. La probabilidad de encuentro por paso evita sequías y se adapta al terreno del mapa. La FSM desacopla los estados del juego y prepara el terreno para agregar inventario, tiendas, etc.

### Log 10/06/2026 — Correcciones de bugs, calidad y estructura

#### Cambios realizados

| # | Cambio | Archivos afectados |
|---|--------|--------------------|
| 1 | **Crash `mostrarEstado`** — `armaEquipada` se accedía sin verificar `nullptr`. Agregado guard condition y mensaje "ninguna" si no hay arma equipada. | `src/Jugador.cpp` |
| 2 | **Variable centralizada `dataPath()`** — Los JSON estaban en `json/` pero el código buscaba en la raíz. Se creó `dataPath(filename)` en `Batalla.hpp` que antepone `"json/"`. Actualizadas las 7 referencias a archivos. | `lib/Batalla.hpp`, `src/main.cpp`, `src/batalla.cpp` |
| 3 | **Condición de victoria** — Al vencer al jefe final, `batalla()` hacía `return` temprano pero `main.cpp` seguía en el `while`, generando batallas infinitas. Se agregó flag `haGanado` a `Jugador`; ahora `batalla()` setea `setHaGanado(true)` y fluye normalmente; `main.cpp` verifica `!jugador.getHaGanado()` en el loop. | `lib/Jugador.hpp`, `src/batalla.cpp`, `src/main.cpp` |
| 4 | **Naming inconsistente** — `setloot` → `setLoot` (convención camelCase como el resto del código). | `lib/Enemigo.hpp`, `src/Enemigo.cpp` |
| 5 | **Librería `<random>`** — Reemplazado `std::rand()` / `std::srand()` por `std::mt19937` + `std::uniform_int_distribution`. Eliminados `#include <cstdlib>` y `<ctime>` donde ya no eran necesarios. | `src/batalla.cpp`, `src/main.cpp` |
| 6 | **Makefile** — Agregados flags `-Wall -Wextra -Wpedantic -I.`, target con `.exe`, `clean` compatible con Windows. | `Makefile` |
| 7 | **Advertencia `-Wdeprecated-copy`** — `Personaje` tenía copy constructor pero no copy assignment operator. Agregado `operator=(const Personaje&) = default;`. Compilación limpia sin warnings. | `lib/Personaje.hpp` |
| 8 | **Log de cambios** — Este archivo fue limpiado y reestructurado. | `docs/cambios.md` |

#### Por qué se hicieron

**1. Crash `mostrarEstado`**
`armaEquipada` se inicializa en `nullptr` y solo se asigna al equipar un arma. Si el usuario abría la pantalla de estado (opción 4 del menú) sin haber equipado "Espada Gallo" o si `objetos.find("Espada Gallo")` fallaba, la línea `armaEquipada->getNombre()` producía un segfault. `mostrarInventario()` ya tenía el guard condition, pero `mostrarEstado()` no.

**2. Paths de JSON**
Los archivos `objetos.json`, `enemigos.json` y `heroe.json` estaban en `json/` pero el código abría `"objetos.json"` desde la raíz, fallando siempre. La función `dataPath()` centraliza el prefijo `"json/"` en un solo lugar para evitar tener que buscar todas las llamadas.

**3. Condición de victoria**
El jefe final (nivel 4) se vencía pero `batalla()` retornaba sin indicarle a `main()` que el juego terminó. `main()` seguía en `while(jugador.estaVivo())` y generaba otro enemigo del nivel 4, creando un loop infinito. Con `haGanado`, el loop también verifica el flag de victoria.

**4. Naming**
`setloot` rompía la convención camelCase del resto del código (`getLoot1`, `getLoot2`, `getNombre`, `getSalud`, etc.).

**5. `<random>` vs `std::rand()`**
`std::rand()` tiene periodo pequeño, distribución pobre y no es thread-safe. `std::mt19937` es el estándar moderno de C++ para generación de números pseudoaleatorios.

**6. Makefile**
Faltaban `-Wall -Wextra -Wpedantic` para detectar warnings en compilación. Faltaba `-I.` para búsqueda de headers. El `clean` usaba `rm -rf` que no funciona en Windows nativo.

**7. Warning de compilación**
`Personaje` declaraba copy constructor pero no copy assignment. En C++11 la generación implícita del copy assignment está deprecada, generando warning con `-Wdeprecated-copy`.

#### Magic Numbers: Explicación

Los **magic numbers** son valores numéricos sin nombre que aparecen hardcodeados sin contexto:

| Ubicación | Valor | ¿Qué representa? |
|-----------|-------|------------------|
| `src/Jugador.cpp:7` | 100, 15, 10, 1, 3 | Stats base del héroe |
| `src/Jugador.cpp:19` | 30 | Curación de poción |
| `src/Jugador.cpp:132` | 50 | Multiplicador de salud por nivel |
| `src/Jugador.cpp:134-135` | 5 | Incremento de ataque/defensa por nivel |
| `src/Jugador.cpp:137` | 200 | Umbral de XP adicional |
| `src/Jugador.cpp:138` | 700 | XP necesaria en nivel 3 |
| `src/batalla.cpp:87-88` | 70, 30 | Probabilidades de loot (hardcodeadas, ignorando `prob1`/`prob2` del JSON) |
| `src/batalla.cpp:206` | 50 | XP ganada por batalla |

**Problemas concretos:**

- Si quieres cambiar la curación de poción, tienes que acordarte que 30 es el número mágico en ``Jugador.cpp:19`` — no hay forma de buscarlo semánticamente
- Si quieres balancear el sistema de nivelación, hay 6 números dispersos en la función ``obtenerExperiencia`` sin relación explícita entre ellos
- Si el nivel 3 tiene un tratamiento especial (``expNecesaria = 700``), ese 3 y 700 están aislados sin documentar por qué
- El 50 de XP por batalla (``jugador.getNivel() * 50``) aparece en ``batalla.cpp`` sin conexión con los multiplicadores de nivelación en ``Jugador.cpp``
**La solución** es reemplazarlos con constantes con nombre:
```cpp
// Antes:
saludMaxima += 50 * (nivel + 1);

// Despues:
constexpr int SALUD_POR_NIVEL = 50;
saludMaxima += SALUD_POR_NIVEL * (nivel + 1);
```
Esto hace que el código sea **autodocumentado**, más fácil de **balancear** (todos los números están etiquetados), y evita errores al cambiar un valor sin querer que coincida con otro idéntico.

- Cambiar la curación de poción requiere saber que `30` en `Jugador.cpp:19` es ese valor — no hay forma de buscarlo semánticamente
- Hay 6 números dispersos en `obtenerExperiencia` sin relación explícita
- El nivel 3 tiene tratamiento especial (`expNecesaria = 700`) sin documentar por qué
- El `50` de XP por batalla no tiene conexión con los multiplicadores de nivelación

**Solución pendiente:** Reemplazar con `constexpr` con nombre como `SALUD_POR_NIVEL`, `POCION_CURACION`, `XP_POR_BATALLA`, etc.

#### 8. Críticas en diseño (detalladas)
**God Class ``Jugador``**

``Jugador`` maneja **7 responsabilidades** distintas: combate (``atacar``), inventario (``agregarObjeto, eliminarObjeto, mostrarInventario``), equipo (``equiparArma``), UI/menús (``mostrarMenu, mostrarEstado``), nivelación (``obtenerExperiencia``), curación (``usarPocion``, dos sobrecargas) y ahora persistencia indirecta.

Maneja 7 responsabilidades: combate (`atacar`), inventario (`agregarObjeto`, `eliminarObjeto`, `mostrarInventario`), equipo (`equiparArma`), UI (`mostrarMenu`, `mostrarEstado`), nivelación (`obtenerExperiencia`), curación (`usarPocion` dos sobrecargas) y persistencia indirecta. ~37 líneas de declaración y 145 de implementación. Extraer `Inventario` como clase separada simplificaría `Jugador` significativamente.


**Consecuencia**: la clase tiene ~37 líneas de declaración y 145 de implementación, todo mezclado. Para añadir una armadura equipable tendrías que tocar 4 métodos distintos en la misma clase porque no hay separación de conceptos. La clase ``Inventario`` debería manejarse por separado.

**God File ``batalla.cpp``**
Combina:
- Parsing JSON de objetos
- Parsing JSON de enemigos
- Carga/guardado de héroe
- Generación aleatoria de enemigos
- Lógica completa de la batalla
- Utilidades de I/O (limpiarBuffer, limpiarPantalla)

Combina parsing JSON, lógica de batalla, persistencia y utilidades I/O. Un cambio en la lectura de JSON puede romper la batalla y viceversa. Dividir en `DataLoader.cpp`, `SaveManager.cpp`, `Combate.cpp`, `Utils.cpp`.

**Problema**: cualquier cambio en cómo se leen los JSON puede romper la batalla, y viceversa. Idealmente debería dividirse en ``DataLoader.cpp`` (JSON), ``SaveManager.cpp`` (persistencia), ``Combate.cpp`` (lógica de batalla), ``Utils.cpp`` (I/O).

**UI acoplada a lógica de negocio**
Todos los std::cout y std::cin están incrustados en las clases de modelo (Jugador::atacar, Enemigo::atacar, Personaje::recibirDano).

**Consecuencia**: no puedes escribir **tests unitarios** sin capturar stdout. Si quisieras migrar a ncurses, SDL o una interfaz web, tendrías que reescribir cada función. La solución es separar en una interfaz Output/Input que las clases de modelo reciban por inyección.
`std::cout`/`std::cin` directos en `Jugador::atacar`, `Enemigo::atacar`, `Personaje::recibirDano`. Imposible testear unitariamente sin capturar stdout. Separar en interfaz `Output`/`Input` por inyección.

**``shared_ptr`` sobreutilizado**
``Objeto.hpp:49`` define ``Drop`` con ``shared_ptr<Objeto>``, y el mapa objetosInventario también usa ``shared_ptr``. Esto introduce conteo de referencias atómico innecesario. ``Objeto, Arma, Pocion, ObjClave`` tienen dueño único (el inventario del jugador o el mapa global de objetos cargados). Bastaría ``unique_ptr`` para los mapas y raw pointers (no owning) para los ``Drop``.
`Drop` con `shared_ptr<Objeto>` introduce conteo de referencias atómico innecesario. Los objetos tienen dueño único. Bastaría `unique_ptr` para los mapas y raw pointers (no owning) para los `Drop`.

**Sin tests**
No hay ningún test. Con la UI acoplada, probar la lógica de combate requiere ejecutar el juego completo y leer la salida por terminal. Separar la lógica de la presentación permitiría testear:

- Fórmulas de daño (``recibirDano``)
- Sistema de nivelación (``obtenerExperiencia``)
- Probabilidades de loot
- Algoritmo de selección de enemigos

**Manejo de errores**
Las funciones JSON lanzan `std::runtime_error` pero `main.cpp` no tiene ningún `try/catch`. Cualquier error (objeto faltante, JSON mal formado, nivel sin enemigos) llama a `std::terminate()` sin mensaje al usuario.

Las funciones JSON lanzan excepciones (``throw std::runtime_error``) pero ``main.cpp`` no tiene un solo ``try/catch``. Si:

- Un objeto del JSON de enemigos no existe → ``terminate()``
- No hay enemigos para el nivel del jugador → ``terminate()``
- ``heroe.json`` está mal formado → ``terminate()``
Tres formas distintas de crashear sin mensaje de error al usuario.
