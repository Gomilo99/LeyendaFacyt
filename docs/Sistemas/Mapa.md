# Sistema de Mapa y FSM del Juego #sistema/mapa

> Archivos: `lib/Mapa.hpp`, `src/Mapa.cpp`, `lib/GameManager.hpp`, `src/GameManager.cpp`

El mapa es una matriz 2D cargada desde archivos `.txt`. `GameManager` orquesta la máquina de estados principal del juego (FSM) y gestiona el movimiento, eventos de tiles y transiciones entre estados.

Ver también: [[Enemigos]] (encuentros aleatorios al moverse), [[Combate]] (se invoca desde OVERWORLD), [[Guardado]] (persiste estado del mapa y héroe).

---

## Máquina de estados (FSM)

```
MAIN_MENU → OVERWORLD → (BATTLE anidado) → OVERWORLD o GAME_OVER
```

### Estados

| Estado | Descripción |
|--------|-------------|
| `MAIN_MENU` | Menú principal: Nueva Partida / Continuar / Salir |
| `OVERWORLD` | Exploración: renderizado top-down, movimiento WASD, eventos de tiles, encuentros |
| `GAME_OVER` | Jugador murió, fin de partida |

El estado BATTLE es síncrono y anidado dentro de OVERWORLD: `iniciarCombate()` llama a `batalla()` que bloquea hasta que termina el combate.

### MAIN_MENU

| Opción | Acción |
|--------|--------|
| **1. Nueva Partida** | `CacheManager::limpiar()` → carga mapa original + héroe default → `CacheManager::crearPartida()` → OVERWORLD |
| **2. Continuar** | Si existe `cache/partida.flag`: carga mapa y héroe desde [[Guardado|caché]]. Si no, muestra mensaje y vuelve al menú. |
| **3. Salir** | Termina el juego |

---

## Mapa — Clase entidad

### Carga y representación

El mapa se carga desde archivos `.txt` en `mapas/`. Cada carácter representa un tile:

| Símbolo | Significado |
|---------|-------------|
| `#` | Pared — no transitable |
| `.` | Suelo — transitable |
| `P` | Posición inicial del jugador |
| `E` | Spawn de enemigo (obsoleto, reemplazado por encuentros aleatorios) |
| `B` | Jefe final del nivel |
| `K` | Llave mágica (victoria) |
| `H` | Poción en el suelo |

### API

```cpp
class Mapa {
    void cargar(const string& archivo);    // Carga desde .txt
    void guardar(const string& archivo);   // Serializa a .txt
    char getTile(int x, int y);            // Lee tile en posición
    void setTile(int x, int y, char tile); // Modifica tile
    bool esTransitable(int x, int y);      // true si no es pared
    int getAncho();                        // Ancho del mapa
    int getAlto();                         // Alto del mapa
};
```

### Mapas actuales

| Archivo | Dimensiones | Descripción |
|---------|-------------|-------------|
| `mapas/nivel1.txt` | 16x11 | Habitación abierta con P, K, B, H |
| `mapas/nivel2.txt` | 20x14 | Laberinto complejo con múltiples habitaciones |

> **Nota**: Solo nivel1.txt es accesible actualmente. No hay transición entre niveles. Ver [[Planificacion/Roadmap#Objetivo 50%]].

---

## Tiles especiales — Flujo de eventos

```
OVERWORLD
  │
  ├─ WASD ──────────→ mover jugador
  │                     │
  │                     ├─ tile 'B' ──→ [[Enemigos|crearJefe(nivel)]] → [[Combate|batalla()]]
  │                     │               ├─ victoria → mapa.setTile('.') → [[Guardado|guardarMapa()]]
  │                     │               └─ derrota  → GAME_OVER
  │                     ├─ tile 'K' ──→ haGanado = true
  │                     ├─ tile 'H' ──→ usar poción → tile → '.' → [[Guardado|guardarMapa()]]
  │                     └─ tile '.' ──→ [[Enemigos|EncounterManager::checkEncounter()]]
  │                                       │
  │                                       ├─ true  → crearEnemigo(nivel) → [[Combate|batalla()]]
  │                                       │           └─ victoria → OVERWORLD
  │                                       │           └─ muerte  → GAME_OVER
  │                                       │
  │                                       └─ false → OVERWORLD (sigue)
  │
  └─ 'Q' ──→ [[Guardado|guardarPartida()]] → salir
```

---

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
  │     │     ├── Mapa::cargar("mapas/nivel1.txt")
  │     │     ├── Jugador("Heroe") + equipar "Espada Gallo"
  │     │     └── CacheManager::crearPartida()
  │     │
  │     ├─ Opción 2 (Continuar):
  │     │     ├── CacheManager::cargarMapa()
  │     │     └── CacheManager::cargarHeroe(objetos)
  │     │
  │     └─ Opción 3 (Salir) → return
  │
  └── OVERWORLD:
        ├── tile '.' + encounter → EnemyFactory → [[Combate|batalla()]]
        ├── tile 'B' → EnemyFactory → [[Combate|batalla()]]
        ├── tile 'H' → usar poción → setTile('.')
        ├── tile 'K' → victoria
        └── 'Q' → [[Guardado|guardar]] → salir
```

---

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

## Controles

| Tecla | Acción |
|-------|--------|
| **1/2/3** | Menú principal: Nueva Partida / Continuar / Salir |
| **W/A/S/D** | Moverse (arriba/izquierda/abajo/derecha) |
| **I** | Abrir [[Inventario]] |
| **Q** | [[Guardado|Guardar]] partida y salir del juego |
| **Enter** | Ir al menú desde pantalla de título |
