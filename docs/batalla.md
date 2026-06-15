# Sistema de Combate

## Arquitectura

```
lib/batalla.hpp          ← declaraciones de clases y funciones
src/batalla.cpp          ← implementación completa
```

### Clases principales

**`ScreenBuffer`** — Búfer de pantalla (22×56)
- Mantiene una matriz `char[22][56]` como doble búfer
- Métodos: `setChar`, `drawString`, `drawHLine`, `drawVLine`, `drawBox`, `drawBar`
- `render()`: limpia la terminal con `\033[2J\033[1;1H` y vuelca el búfer completo

**`Renderer`** — Dibuja cada frame del combate
- Recibe una referencia al `ScreenBuffer`
- `drawBackground()`: encabezado "LEYENDA DEL CAMPUS - COMBATE"
- `drawEnemy()`: recuadro con nombre del enemigo + arte ASCII (6 líneas)
- `drawEnemyHealthBar()`: barra de vida con `#` y texto `HP: X/Y`
- `drawCombatMenu()`: menú de 4 opciones (Atacar / Magia / Inventario / Huir) con selector `>`
- `drawPlayerInfo()`: panel del jugador con nombre, HP bar, MP bar y valores numéricos
- `drawLog()`: mensaje de estado en la última línea

**`InputHandler`** — Navegación del menú
- 4 opciones, navegación circular con `moveUp()` / `moveDown()`

**`BattleSystem`** — Máquina de estados del combate
- Estados: `PLAYER_TURN → PLAYER_ACTION → ENEMY_TURN → (loop) → VICTORY | DEFEAT | FLEE`
- `run()`: bucle principal
- `processInput()`: lee teclas W/S/SPACE vía `std::cin.get()`
- `doPlayerAction()`: ejecuta la opción seleccionada
- `doEnemyTurn()`: el enemigo ataca al jugador
- `generateEnemyArt()`: elige arte ASCII según keywords en el nombre del enemigo

### `enum class BattleState`

| Estado         | Descripción                                    |
|----------------|------------------------------------------------|
| `PLAYER_TURN`  | Esperando entrada del jugador (W/S/SPACE)      |
| `PLAYER_ACTION`| Ejecutando la acción seleccionada              |
| `ENEMY_TURN`   | Turno del enemigo (con pausa de 300ms)         |
| `ANIMATION`    | Reservado para animaciones futuras             |
| `VICTORY`      | Enemigo derrotado                               |
| `DEFEAT`       | Jugador sin HP                                  |
| `FLEE`         | Jugador huyó del combate                       |

---

## Flujo del combate

### 1. Transición desde el mapa
```
main.cpp: tile 'E' → batalla(jugador, enemigos)
  └─ generarEnemigoPorNivel() según nivel del jugador
  └─ Enemigo enemigo(*enemigoSelec)  ← copia del template
  └─ BattleSystem system(jugador, enemigo)
  └─ system.run()
```

### 2. Bucle de combate (`BattleSystem::run()`)
```
PLAYER_TURN
  ├─ render()  → dibuja frame completo
  ├─ processInput()
  │   ├─ W/S → moveUp/moveDown (navegación)
  │   └─ SPACE → confirma → PLAYER_ACTION
  │
  └─ doPlayerAction()
      ├─ [0] Atacar:  player.atacar(enemigo)
      ├─ [1] Magia:   player.usarMagia(enemigo)  (cuesta 10 MP)
      ├─ [2] Inventario: muestra inventario, pausa, vuelve a PLAYER_TURN
      └─ [3] Huir:    50% de éxito → FLEE, si falla → ENEMY_TURN

ENEMY_TURN
  └─ doEnemyTurn()
      ├─ pausa 300ms
      ├─ enemigo.atacar(jugador)
      └─ si jugador muere → DEFEAT, si no → PLAYER_TURN

VICTORY → battleOver = true
DEFEAT  → battleOver = true
FLEE    → battleOver = true
```

### 3. Post-combate (`batalla()`)
```
system.run() termina:

if (fled)        → vuelve al mapa sin cambios
if (jugador muerto) → guardarHeroe(), GAME OVER

// Victoria
├─ mostrar "HAS DERROTADO A ..."
├─ si era jefe final (nivel ≥ 4) → jugador.setHaGanado(true)
├─ experiencia: nivel * 50
├─ loot: random según probabilidad del enemigo (loot1.probabilidad / loot2.probabilidad)
└─ guardarHeroe()
```

---

## Layout visual

```
=== LEYENDA DEL CAMPUS - COMBATE ===   ← y=0 (Background)
──────────────────────────────          ← y=1
+================+                     ← y=2 (caja nombre enemigo)
|     SLIME      |                     ← y=3
+================+                     ← y=4
HP: 40/40                              ← y=4 (texto vida)
████████████████████░░░░░░░░░░░░       ← y=5 (barra vida 30 chars)
     .-.                               ← y=6 (arte enemigo)
    (o o)                              ← y=7
    | ~ |                              ← y=8
    \___/                              ← y=9
   /     \                             ← y=10
  /       \                            ← y=11
+--------------------+  +--------------------------+
|  > Atacar          |  | Heroe                   | ← y=12-13
|    Magia           |  | HP ############....      |
|    Inventario      |  | MP ######........        |
|    Huir            |  | 45/60                    |
| [W/S] Navegar [OK] |  | 22/40                    | ← y=17
+--------------------+  +--------------------------+ ← y=18
                                                     ← y=19 (vacío)
                Heroe ataca a Slime!                ← y=21 (log)
```

---

## Arte de enemigos

El arte se genera automáticamente según keywords en el nombre:

| Keyword                    | Estilo                      |
|----------------------------|-----------------------------|
| dragon, admin              | Dragón / Boss               |
| golem, ogro                | Golem rocoso                |
| fantasma, espectro, bruja  | Fantasma flotante           |
| esqueleto                  | Esqueleto                   |
| cajero                     | Cajero automático           |
| gargola, caballero         | Caballero con armadura      |
| ciclope                    | Ciclope                     |
| slime                      | Slime                       |
| goblin, duende             | Goblin                      |
| orco                       | Orco                        |
| zombie, murcielago         | Zombie                      |
| default                    | Forma genérica              |

---

## Sistema de maná

- El jugador tiene `mana` y `manaMaxima` (incial: 50 + nivel × 10)
- `usarMagia()`: consume 10 MP, causa `ataque × 2 + nivel × 5` de daño
- Si no hay MP suficiente, se muestra mensaje y se regresa al menú
- El maná se guarda en `heroe.json` con el resto de stats

---

## Archivos modificados

| Archivo                 | Cambio                                                   |
|-------------------------|----------------------------------------------------------|
| `lib/batalla.hpp`       | Nuevas clases: ScreenBuffer, Renderer, InputHandler, BattleSystem + enum BattleState |
| `src/batalla.cpp`       | Implementación completa del sistema de combate con renderizado |
| `lib/jugador.hpp`       | Añadidos: mana, manaMaxima, getMana(), getManaMaxima(), setMana(), usarMagia() |
| `src/jugador.cpp`       | Implementación de mana, magic attack, mostrarEstado/mostrarMenu actualizados |
| `lib/personaje.hpp`     | Añadido getSaludMaxima()                                 |
| `src/main.cpp`          | Movimiento corregido (setPos después de esTransitable), tiles E/H se limpian |
| `Makefile`              | Actualizado a -std=c++17                                 |
