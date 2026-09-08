---
creado: 22/07/2026
modificado: 22/07/2026
tipo: Avance
tags: # deuda-tecnica, idea-loca, bug-critico, bug, refactor
titulo: Arquitectura del Sistema
proyecto: "[[LeyendaFacyt]]"
area: Base
estado: En progreso
prioridad: 2
dificultad: Media
version: 1.0.0
---
# Arquitectura del Sistema

Visión general de la arquitectura técnica de [[LeyendaFacyt]]. Para detalles de cada sistema, ver la documentación específica en [[Sistemas/Combate|Sistemas]].

---

## Managers principales

| Manager | Responsabilidad | Doc |
|---------|----------------|-----|
| **GameManager** | Orquestador del juego con FSM. Menú principal, loop de exploración, renderizado del mapa, movimiento WASD, eventos de tiles, inicio de combate | [[Sistemas/Mapa]] |
| **DataManager** | Carga datos desde JSON (`objetos.json`, `enemigos.json`). Solo lectura | [[Sistemas/Guardado]] |
| **CacheManager** | Capa de persistencia en `cache/`. Guarda/carga héroe (14 campos + inventario), mapa y flag de partida | [[Sistemas/Guardado]] |
| **EnemyFactory** | Carga `json/enemigos.json`, crea enemigos con selección ponderada por `peso` | [[Sistemas/Enemigos]] |
| **EncounterManager** | Decide encuentros aleatorios al moverse usando probabilidad por terreno | [[Sistemas/Enemigos]] |

## Diagrama de dependencias

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
Platform.hpp  → (standalone, includes del SO)
main.cpp      → GameManager.hpp
```

## Jerarquía de clases

```
Personaje (abstracta)
  ├── Jugador
  │     ├── inventario (map<string, int>)
  │     ├── objetosInventario (map<string, shared_ptr<Objeto>>)
  │     ├── armaEquipada (shared_ptr<Arma>)
  │     └── posX, posY
  └── Enemigo
        ├── id (string)
        ├── asciiArt[6] (string)
        └── botin (vector<Drop>)

Objeto
  ├── Arma (dano)
  ├── Pocion (curacion)
  └── ObjClave (sin uso)

struct Drop { shared_ptr<Objeto> objeto; int probabilidad; }
```

## Flujo principal del juego

```
main.cpp → GameManager::run()
  │
  ├── Constructor:
  │     ├── DataManager::cargarObjetos()
  │     ├── EnemyFactory::cargarDesdeJSON()
  │     └── Buscar spawn 'P' en mapa
  │
  ├── MAIN_MENU:
  │     ├─ Nueva Partida → CacheManager::limpiar() → Mapa::cargar() → crearPartida()
  │     ├─ Continuar → CacheManager::cargarMapa() + cargarHeroe()
  │     └─ Salir → return
  │
  └── OVERWORLD:
        ├── WASD → moverJugador() → handleTile() → EncounterManager
        ├── tile 'B' → jefe de la zona → derrota → habilita K
        ├── tile 'h/H/G' → curación porcentual → setTile('.')
        ├── tile 'K' → carga el siguiente mapa (o victoria final)
        ├── 'I' → [[Sistemas/Inventario|InventoryUI]]
        └── 'Q' → CacheManager::guardar() → salir
```

## Flujo de datos

```
json/objetos.json  ──lee──▶  DataManager  ──carga──▶  Jugador, Objetos
json/enemigos.json ──lee──▶  EnemyFactory ──crea──▶   Enemigo
mapas/nivel1.txt   ──lee──▶  Mapa

cache/heroe.json   ◀──escribe── CacheManager ◀──recibe── Jugador
cache/mapa_cache.txt ◀──escribe── CacheManager ◀──recibe── Mapa
cache/partida.json ◀──escribe── CacheManager ◀──recibe── estado de campaña
```

## Controles

| Tecla | Contexto | Acción |
|-------|----------|--------|
| **1/2/3** | Menú | Nueva Partida / Continuar / Salir |
| **W/A/S/D** | Overworld | Moverse |
| **I** | Overworld | Abrir [[Sistemas/Inventario]] |
| **Q** | Overworld | [[Sistemas/Guardado\|Guardar]] y salir |
| **Enter** | Título | Ir al menú |
| **W/S** | Combate | Navegar opciones |
| **SPACE** | Combate/Inventario | Confirmar |
| **A/D** | Inventario | Cambiar categoría |
| **Q** | Inventario | Cerrar |

## Artefactos de build

| Target | Comando | Descripción |
|--------|---------|-------------|
| Debug | `make` | Build con debug → `leyenda.exe` |
| Dist | `make dist` | Optimizado (-O2), estático → `.dist/leyenda.exe` |
| Run | `make run` | Build + ejecutar |
| Clean | `make clean` | Limpiar artefactos |

Ver [[Sistemas/Plataforma]] para detalles de compilación cross-platform.
