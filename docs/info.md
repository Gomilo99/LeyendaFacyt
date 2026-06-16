## Visión general del juego

**Leyenda del Campus** es un dungeon crawler por turnos con interfaz CLI, arte ASCII y colores ANSI. El jugador explora un mapa 2D top-down, enfrenta enemigos aleatorios en combate por turnos, recolecta objetos y sube de nivel hasta derrotar al jefe final.

## Arquitectura del sistema

### Managers principales

| Manager | Responsabilidad |
|---|---|---|
| **GameManager** | Orquestador del juego con FSM (`MAIN_MENU → OVERWORLD → GAME_OVER`). Loop principal, renderizado del mapa, movimiento WASD, eventos de tiles, inicio de combate, condición de victoria |
| **DataManager** | Carga/guarda toda la data desde JSON (objetos, enemigos, héroe). Usa `Config` para paths |
| **CacheManager** | Maneja archivos de cache (`cache/`) para persistir el progreso sin modificar archivos originales |
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

### heroe.json

```json
{ "nombre": "...", "salud": 100, "ataque": 15, "defensa": 10, "nivel": 1, "pociones": 3, "mana": 50 }
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

1. **MAIN_MENU**: Pantalla de título con instrucciones, espera Enter
2. **OVERWORLD**: Loop de exploración con renderizado top-down del mapa, movimiento WASD, eventos de tiles, encuentros aleatorios
3. **GAME_OVER**: Jugador murió, fin de partida

El estado BATTLE es síncrono y anidado dentro de OVERWORLD: `iniciarCombate()` o `iniciarCombateJefe()` llaman a `batalla()`, que bloquea hasta que termina el combate.

### Flujo completo por paso

```
OVERWORLD
  │
  ├─ WASD ──────────→ mover jugador
  │                     │
  │                     ├─ tile 'B' ──→ crearJefe(nivel) → batalla()
  │                     ├─ tile 'K' ──→ haGanado = true
  │                     ├─ tile 'H' ──→ usar poción, tile → '.'
  │                     └─ tile '.' ──→ EncounterManager::checkEncounter()
  │                                       │
  │                                       ├─ true  → crearEnemigo(nivel) → batalla()
  │                                       │           │
  │                                       │           └─ victoria → OVERWORLD
  │                                       │           └─ muerte  → GAME_OVER
  │                                       │
  │                                       └─ false → OVERWORLD (sigue)
  │
  └─ 'Q' ──→ salir
```

## Flujo de datos

```
main.cpp → GameManager::run()
  │
  ├── Constructor:
  │     ├── DataManager::cargarObjetos()     → json/objetos.json
  │     ├── DataManager::cargarEnemigos()    → json/enemigos.json
  │     ├── DataManager::cargarHeroe()       → json/heroe.json
  │     ├── Mapa::cargar()                   → mapas/nivel1.txt
  │     ├── EnemyFactory (usa enemigos.json internamente)
  │     └── equipar arma inicial "Espada Gallo"
  │
  └── loop (OVERWORLD):
        ├── tile '.' + encuentro random → EnemyFactory::crearEnemigo() → batalla()
        ├── tile 'B' → EnemyFactory::crearJefe() → batalla()
        ├── tile 'H' → jugador.usarPocion()
        ├── tile 'K' → victoria
        └── 'Q' → salir
```

## Dependencias entre archivos

```
GameManager → DataManager, batalla.hpp, mapa.hpp, jugador.hpp,
              enemyFactory.hpp, encounterManager.hpp
DataManager → Config, json.hpp, objeto.hpp, enemigo.hpp, jugador.hpp
EnemyFactory → Config, json.hpp, enemigo.hpp, objeto.hpp
EncounterManager → (standalone, solo random)
CacheManager → Config, json.hpp  (independiente del combate)
batalla.hpp → enemigo.hpp, jugador.hpp  (ya NO incluye json.hpp)
main.cpp → GameManager.hpp
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
| **W/A/S/D** | Moverse (arriba/izquierda/abajo/derecha) |
| **I** | Abrir inventario |
| **Q** | Salir del juego |
| **Enter** | Comenzar desde pantalla de título |
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
    ├── heroe.json     ← guardado del héroe
    ├── mapa_cache.txt ← estado actualizado del mapa (con E/H marcados como .)
    └── partida.json   ← metadatos de la partida
```

#### Flujo de carga
main():
1. Cargar JSONs desde json/ (originales, read-only)
2. Cargar mapa desde mapas/nivel1.txt (original, read-only)
3. Si existe cache/heroe.json → cargar héroe desde caché
4. Si existe cache/mapa_cache.txt → cargar mapa desde caché
   (así se conservan los cambios: E eliminados, H recogidas)
5. Si no existe caché → copiar originales a caché

#### Cómo implementar la caché del mapa
Paso 1 — Crear **CacheManager** (``lib/CacheManager.hpp``)
```cpp
namespace CacheManager {
    bool existePartida();
    void crearPartida(const Mapa& mapa, const Jugador& jugador);
    
    // Mapa
    bool guardarMapa(const Mapa& mapa);
    bool cargarMapa(Mapa& mapa);  // si falla, cargar del original
    
    // Héroe
    void guardarHeroe(const Jugador& jugador);
    Jugador cargarHeroe();
    
    // Limpiar caché (para nueva partida)
    void limpiar();
}
```
Paso 2 — Flujo en ``main()``
```
1. Cargar archivos originales (JSON, mapa)
2. if (existePartida()) {
       cargarMapa desde caché      → refleja E/H ya gastados
       cargarHeroe desde caché
   } else {
       crearPartida()              → copia originales a caché
   }
3. Bucle de juego:
   - Al vencer enemigo en 'E':
       mapa.setTile(x, y, '.')     → modifica en memoria
       CacheManager::guardarMapa(mapa)  → persiste el cambio
   - Al recoger poción en 'H':
       mapa.setTile(x, y, '.')
       CacheManager::guardarMapa(mapa)
       jugador.usarPocion()
   - Al salir del juego o girar turno:
       CacheManager::guardarHeroe(jugador)
```

Paso 3 — Formato de la caché del mapa

El ``mapa_cache.txt`` puede ser idéntico al original (``vector<string>``), solo que con los tiles modificados. La clase ``Mapa`` ya tiene ``getTile/setTile``. Solo falta un método ``guardar(archivo)``:
```cpp
// En Mapa.hpp
bool guardar(const std::string& archivo) const;

// En Mapa.cpp
bool Mapa::guardar(const std::string& archivo) const {
    std::ofstream file(archivo);
    if (!file.is_open()) return false;
    for (const auto& linea : grid) {
        file << linea << '\n';
    }
    return true;
}
```
3. Cuándo se guarda cada cosa
Evento	Se guarda en caché
Derrotar enemigo (tile 'E' → '.')	mapa_cache.txt
Recoger poción (tile 'H' → '.')	mapa_cache.txt
Cambiar de nivel / sala	mapa_cache.txt + heroe.json
Recibir daño en mapa (trampa 'T')	heroe.json
Subir de nivel en combate	heroe.json (ya lo hace guardarHeroe en batalla)
Salir del juego con 'Q'	heroe.json + mapa_cache.txt

> **Nota:** El sistema de caché (`CacheManager`) está implementado pero actualmente el juego carga/guarda directamente desde `json/heroe.json` sin pasar por la capa de caché. La integración completa (cargar desde `cache/` si existe partida guardada) está pendiente.