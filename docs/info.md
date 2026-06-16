## Visión general del juego

**Leyenda del Campus** es un dungeon crawler por turnos con interfaz CLI, arte ASCII y colores ANSI. El jugador explora un mapa 2D top-down, enfrenta enemigos aleatorios en combate por turnos, recolecta objetos y sube de nivel hasta derrotar al jefe final.

## Arquitectura del sistema

### Managers principales

| Manager | Responsabilidad |
|---|---|---|
| **GameManager** | Orquestador del juego con FSM (`MAIN_MENU → OVERWORLD → GAME_OVER`). Menú principal (Nueva Partida/Continuar/Salir), loop de exploración, renderizado del mapa, movimiento WASD, eventos de tiles, inicio de combate, condición de victoria, guardado al salir |
| **DataManager** | Carga datos desde JSON (`objetos.json`, `enemigos.json`). Solo lectura. Usa `Config` para paths. Ya no guarda ni carga héroe (esa función pasó a `CacheManager`) |
| **CacheManager** | Capa de persistencia en `cache/`. Guarda/carga héroe (14 campos + inventario), mapa (tiles modificados) y flag de partida. No modifica archivos originales |
| **EnemyFactory** | Carga `json/enemigos.json`, almacena plantillas por nivel en `map<int, vector<EnemyTemplate>>`. Crea enemigos con selección ponderada por `peso` (ruleta). Busca jefes hacia abajo desde el nivel actual |
| **EncounterManager** | Decide encuentros aleatorios al moverse por tile `.` usando probabilidad por terreno (BOSQUE=8%, CAVERNA=10%, MAZMORRA=15%, ABISMO=18%). Grace period de 3 pasos, +3% por paso extra, cap 40% |

### Sistema de combate

El combate es por turnos con interfaz gráfica ASCII en tiempo real:

1. **ScreenBuffer** — Buffer de doble capa (chars + colores ANSI) de 56x22 con redibujado diferencial (solo líneas modificadas se reescriben)
2. **Renderer** — Compone cada frame: fondo, nombre del enemigo + arte ASCII (6 líneas), barra de HP del enemigo (verde/amarillo/rojo según %), menú de 4 opciones, panel del jugador (HP/MP bars), mensaje de log
3. **InputHandler** — Navegación W/S/SPACE por el menú de 4 opciones (selección circular)
4. **BattleSystem** — Máquina de estados: `PLAYER_TURN` → `PLAYER_ACTION` → `ENEMY_TURN` → loop. Estados terminales: `VICTORY`, `DEFEAT`, `FLEE`

### Acciones en combate

| Opción | Descripción |
|---|---|
| **Atacar** | Ataque físico: `jugador.atacar(enemigo)` usando el ataque base + daño del arma equipada |
| **Magia** | Hechizo que cuesta 10 MP. Daño = `ataque * 2 + nivel * 5`. Requiere mínimo 10 MP |
| **Inventario** | Muestra estado y lista de objetos. Permite usar pociones escribiendo el nombre exacto |
| **Huir** | 50% de probabilidad de éxito. Si falla, el enemigo ataca |

### Estadísticas del jugador

| Stat | Descripción |
|---|---|
| **HP** | Salud actual / máxima. Al llegar a 0 el jugador muere |
| **MP** | Maná para hechizos. Base 50 + 10 por nivel |
| **Ataque** | Daño base. Se incrementa con armas equipadas y al subir de nivel |
| **Defensa** | Reduce el daño recibido. El daño real = `max(1, ataque - defensa)` |

### Sistema de nivelación

- Por batalla ganada: XP = `nivel * 50`
- Al alcanzar la XP necesaria: sube de nivel
- Al subir: HP_max += 50*(nivel+1), HP se restaura al máximo, ataque += 5*(nivel+1), defensa += 5*(nivel+1), XP_necesaria += 200
- Caso especial: en nivel 3, XP_necesaria se fija en 700

#### suppressCout — Silenciamiento de salida durante combate

Durante el combate, las funciones `Jugador::atacar()`, `Jugador::usarMagia()`, `Enemigo::atacar()` y `Personaje::recibirDano()` escriben directamente a `std::cout` con mensajes como "recibe X de dano!". Estos mensajes desincronizarían el ScreenBuffer. Para evitarlo:

1. `BattleSystem::suppressCout()` redirige `cout.rdbuf()` a un `ostringstream` interno
2. Se ejecuta la acción de combate (atacar, magia, etc.) — los cout se descartan
3. `BattleSystem::restoreCout()` restaura el buffer original

Esto permite que las clases de personaje no necesiten saber si hay un BattleSystem activo — el silenciamiento es transparente.

## Sistema de loot

Cada enemigo tiene dos objetos de loot con probabilidades independientes (default 70% y 30%). Al derrotarlo:
- Se genera un entero aleatorio [0, 99]
- Si cae dentro de la probabilidad del loot1 → obtiene ese objeto
- Si no, pero cae dentro de loot1 + loot2 → obtiene loot2
- Si es un arma, pregunta si desea equiparla

## Formato de archivos JSON

### objetos.json

```json
{
    "arma": [{ "nombre": "...", "descripcion": "...", "dano": 10 }],
    "pocion": [{ "nombre": "...", "descripcion": "...", "curacion": 30 }],
    "clave": [{ "nombre": "...", "descripcion": "..." }]
}
```

### enemigos.json

```json
{
    "1": [ /* enemigos de nivel 1 */ ],
    "2": [ /* enemigos de nivel 2 */ ],
    "3": [ /* enemigos de nivel 3 */ ]
}
```

Cada enemigo: `nombre`, `salud`, `ataque`, `defensa`, `loot1`/`loot2` (nombre del objeto), `prob1`/`prob2` (probabilidad 0-99, default 70/30).

### heroe.json (archivo original, solo lectura)

```json
{ "nombre": "...", "salud": 100, "ataque": 15, "defensa": 10, "nivel": 1, "pociones": 3, "mana": 50 }
```

### heroe.json (caché, `cache/heroe.json` — estado completo de la partida)

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

## EnemyFactory — Fábrica de enemigos

La `EnemyFactory` carga `json/enemigos.json` y organiza las plantillas por nivel en un `map<int, vector<EnemyTemplate>>`. Cada `EnemyTemplate` contiene: nombre, stats, peso (para selección ponderada), arte ASCII (6 líneas), botín (vector de Drop), y flag de jefe.

### Selección ponderada (ruleta)

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

- Suma todos los `peso` del nivel
- Genera entero en `[0, totalPeso)`
- Itera restando pesos hasta que la tirada llega a 0

### Búsqueda de jefe

`crearJefe(nivel)` busca desde el nivel actual hacia abajo hasta nivel 1. Si no encuentra ningún enemigo con `boss: true`, lanza `std::runtime_error`.

## EncounterManager — Encuentros aleatorios

Al moverse por un tile `.` (sin evento especial), `EncounterManager` decide si ocurre un encuentro:

```
probabilidad = baseTerreno + max(0, pasosSinEncuentro - GRACE_PERIOD) * INCREMENTO_POR_PASO
probabilidad = min(probabilidad, CAP)
```

| Parámetro | Valor |
|---|---|
| GRACE_PERIOD | 3 pasos sin encuentros |
| INCREMENTO_POR_PASO | +3% por paso extra |
| CAP | 40% máximo |
| Terrenos | BOSQUE=8%, CAVERNA=10%, MAZMORRA=15%, ABISMO=18% |

- Pasos 1-3: probabilidad = baseTerreno (sin encuentros forzados)
- Paso 4: +3%, Paso 5: +6%, ... hasta cap 40%
- El contador se resetea al ocurrir un encuentro
- Tiles especiales (B, K, H) skipean el chequeo de encuentro aleatorio

## GameManager — Máquina de estados (FSM)

```
MAIN_MENU → OVERWORLD → (BATTLE anidado) → OVERWORLD o GAME_OVER
```

### MAIN_MENU

El menú principal ofrece 3 opciones:

| Opción | Acción |
|---|---|
| **1. Nueva Partida** | `CacheManager::limpiar()` → carga mapa original + héroe default → `CacheManager::crearPartida()` → OVERWORLD |
| **2. Continuar** | Si existe `cache/partida.flag`: `CacheManager::cargarMapa()` + `CacheManager::cargarHeroe(objetos)` → OVERWORLD. Si no, muestra mensaje y vuelve al menú. |
| **3. Salir** | Termina el juego |

### OVERWORLD

Loop de exploración con renderizado top-down del mapa, movimiento WASD, eventos de tiles, encuentros aleatorios.

### GAME_OVER

Jugador murió, fin de partida.

El estado BATTLE es síncrono y anidado dentro de OVERWORLD: `iniciarCombate()` o `iniciarCombateJefe()` llaman a `batalla()`, que bloquea hasta que termina el combate.

### Flujo completo por paso

```
OVERWORLD
  │
  ├─ WASD ──────────→ mover jugador
  │                     │
  │                     ├─ tile 'B' ──→ crearJefe(nivel) → batalla()
  │                     │               ├─ victoria → mapa.setTile('.') → guardarMapa()
  │                     │               └─ derrota  → GAME_OVER
  │                     ├─ tile 'K' ──→ haGanado = true
  │                     ├─ tile 'H' ──→ usar poción → tile → '.' → guardarMapa()
  │                     └─ tile '.' ──→ EncounterManager::checkEncounter()
  │                                       │
  │                                       ├─ true  → crearEnemigo(nivel) → batalla()
  │                                       │           │
  │                                       │           └─ victoria → OVERWORLD
  │                                       │           └─ muerte  → GAME_OVER
  │                                       │
  │                                       └─ false → OVERWORLD (sigue)
  │
  └─ 'Q' ──→ guardarPartida() → salir
  │           ├─ CacheManager::guardarHeroe()
  │           └─ CacheManager::guardarMapa()
  └─ muerte en batalla ──→ CacheManager::guardarHeroe() → GAME_OVER
```

## Flujo de datos

```
main.cpp → GameManager::run()
  │
  ├── Constructor (solo datos estáticos):
  │     ├── DataManager::cargarObjetos()     → json/objetos.json
  │     ├── EnemyFactory::cargarDesdeJSON()  → json/enemigos.json
  │     └── Buscar spawn 'P' en mapa original
  │
  ├── MAIN_MENU:
  │     ├─ Opción 1 (Nueva Partida):
  │     │     ├── CacheManager::limpiar()
  │     │     ├── Mapa::cargar()             → mapas/nivel1.txt (original)
  │     │     ├── Jugador("Heroe")           → stats base
  │     │     ├── equipar "Espada Gallo"
  │     │     └── CacheManager::crearPartida() → cache/ (flag + mapa + héroe)
  │     │
  │     ├─ Opción 2 (Continuar):
  │     │     ├── CacheManager::cargarMapa() → cache/mapa_cache.txt
  │     │     └── CacheManager::cargarHeroe() → cache/heroe.json (con posición, inventario, arma)
  │     │
  │     └─ Opción 3 (Salir) → return
  │
  └── OVERWORLD:
        ├── tile '.' + encuentro random → EnemyFactory::crearEnemigo() → batalla()
        │                                                     └── CacheManager::guardarHeroe()
        ├── tile 'B' → EnemyFactory::crearJefe() → batalla()
        │               └── victoria → mapa.setTile('.') → CacheManager::guardarMapa()
        ├── tile 'H' → jugador.usarPocion() → mapa.setTile('.') → CacheManager::guardarMapa()
        ├── tile 'K' → victoria
        └── 'Q' → CacheManager::guardarHeroe() + CacheManager::guardarMapa() → salir
```

## Dependencias entre archivos

```
GameManager   → DataManager, CacheManager, batalla.hpp, mapa.hpp,
                jugador.hpp, enemyFactory.hpp, encounterManager.hpp
DataManager   → Config, json.hpp, objeto.hpp, enemigo.hpp, jugador.hpp
CacheManager  → Config, json.hpp, jugador.hpp, mapa.hpp
EnemyFactory  → Config, json.hpp, enemigo.hpp, objeto.hpp
EncounterManager → (standalone, solo random)
batalla.hpp   → enemigo.hpp, jugador.hpp, CacheManager.hpp
Jugador       → Personaje, Objeto
Mapa          → (standalone, solo iostream/fstream)
main.cpp      → GameManager.hpp
```

## Arte ASCII de enemigos

El arte ASCII proviene del campo `ascii` (array de 6 strings) en `json/enemigos.json`. Si un enemigo no tiene `ascii` definido, `BattleSystem::generateEnemyArt()` genera arte por keywords en el nombre:
- `dragon`/`admin` → dragón/BOSS
- `golem`/`ogro` → golem
- `fantasma`/`espectro` → fantasma
- `esqueleto` → esqueleto
- `cajero` → cajero automático
- `gargola`/`caballero` → gargola
- `ciclope` → cíclope
- `slime` → slime
- `goblin`/`duende` → goblin
- `orco` → orco
- `zombie`/`bruja` → zombie
- cualquier otro → forma genérica

## Controles

| Tecla | Acción |
|---|---|
| **1/2/3** | Menú principal: Nueva Partida / Continuar / Salir |
| **W/A/S/D** | Moverse (arriba/izquierda/abajo/derecha) |
| **I** | Abrir inventario |
| **Q** | Guardar partida y salir del juego |
| **Enter** | Ir al menú desde pantalla de título |
| **W/S** (en combate) | Navegar opciones del menú |
| **SPACE** (en combate) | Confirmar opción seleccionada |

## Formato de enemigos.json

Archivo agrupado por nivel, cada enemigo con `peso` (para selección ponderada), `ascii` (6 líneas de arte), `botin` (array de objetos con `nombre` + `probabilidad`), y `boss` (bool):

```json
{
    "1": [
        {
            "nombre": "Goblin",
            "salud": 30, "ataque": 8, "defensa": 3, "nivel": 1,
            "peso": 10,
            "ascii": ["     /\\", "    /  \\", ...],
            "botin": [
                {"nombre": "Pocion Pequena", "probabilidad": 40},
                {"nombre": "Moneda de Cobre", "probabilidad": 60}
            ],
            "boss": false
        }
    ]
}
```

## Leyenda de tiles:

|Símbolo|Significado                |
|-------|---------------------------|
|#      |Pared — no transitable     |
|.      |Suelo — transitable        |
|P      |Posición inicial del jugador|
|E	    |Spawn de enemigo (obsoleto, reemplazado por encuentros aleatorios) |
|B      |Jefe final del nivel       |
|K	    |Llave mágica (victoria)    |
|H	    |Poción en el suelo         |

### Sistema de caché para no modificar archivos originales
#### Concepto
Los archivos originales (``json/objetos.json, enemigos.json, mapas/nivel1.txt``) son read-only para el juego. Cualquier modificación en tiempo de juego se guarda en ``cache/``:

```text
proyecto/
├── json/              ← originales (lectura)
│   ├── objetos.json
│   ├── enemigos.json
│   └── heroe.json     ← se deja de escribir aquí
├── mapas/             ← originales (lectura)
│   └── nivel1.txt
└── cache/             ← generado en tiempo de juego (escritura)
    ├── heroe.json     ← estado completo del héroe (14 campos + inventario)
    ├── mapa_cache.txt ← estado actualizado del mapa (con B/H marcados como .)
    └── partida.flag   ← flag de existencia de partida (archivo vacío)
```

#### Flujo de carga (GameManager — implementación actual)

```
            GameManager::run()
                  │
            mostrarMenuPrincipal()
              ├─ "1. Nueva Partida"
              │     inicializarNuevaPartida()
              │       ├─ CacheManager::limpiar()         → borra cache/
              │       ├─ Mapa::cargar("mapas/nivel1.txt") → original
              │       ├─ Jugador("Heroe") + equipar arma inicial
              │       └─ CacheManager::crearPartida()    → escribe cache/
              │
              ├─ "2. Continuar"
              │     cargarPartidaExistente()
              │       ├─ if (!CacheManager::existePartida()) → mensaje, vuelve al menú
              │       ├─ CacheManager::cargarMapa()         → cache/mapa_cache.txt
              │       └─ CacheManager::cargarHeroe(objetos) → cache/heroe.json
              │
              └─ "3. Salir"
                    → return
```

#### Cuándo se guarda cada cosa (estado actual)

| Evento | Se guarda en caché |
|--------|-------------------|
| Derrotar jefe (tile 'B' → '.') | `guardarMapa()` |
| Recoger poción (tile 'H' → '.') | `guardarMapa()` |
| Terminar combate (victoria) | `guardarHeroe()` (desde `batalla()`) |
| Salir del juego con 'Q' | `guardarHeroe()` + `guardarMapa()` |

#### API de CacheManager

```cpp
namespace CacheManager {
    bool existePartida();                    // cache/partida.flag existe?
    void crearPartida(const Mapa&, const Jugador&);  // flag + mapa + héroe
    void limpiar();                          // borra todo cache/
    
    bool guardarMapa(const Mapa&);           // escribe cache/mapa_cache.txt
    bool cargarMapa(Mapa&);                  // lee cache/mapa_cache.txt
    
    void guardarHeroe(const Jugador&);       // escribe cache/heroe.json
    Jugador cargarHeroe(const map<string, shared_ptr<Objeto>>&);  // lee cache/heroe.json
}
```

#### Formato del mapa en caché

`cache/mapa_cache.txt` es idéntico al original (`vector<string>`), solo que con los tiles modificados (B/H → `.`). Usa `Mapa::guardar(archivo)` que serializa línea por línea.

#### Método especial: `Jugador::agregarObjetoSilencioso()`

Agrega un objeto al inventario sin mostrar el prompt de equipar (necesario al cargar una partida desde caché, donde no hay interacción del usuario).