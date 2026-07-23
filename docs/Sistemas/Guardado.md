---
creado: 22/07/2026
modificado: 22/07/2026
tipo: Avance
tags: # deuda-tecnica, idea-loca, bug-critico, bug, refactor
titulo: Guardado y Formatos de Datos
proyecto: "[[LeyendaFacyt]]"
area: Sistemas
estado: En progreso
prioridad: 2
dificultad: Alta
version: 1.0.0
---
# Sistema de Guardado y Formatos de Datos

> Archivos: `lib/CacheManager.hpp`, `src/cacheManager.cpp`, `lib/DataManager.hpp`, `src/DataManager.cpp`, `lib/config.hpp`, `src/config.cpp`

El juego separa datos originales (solo lectura) del estado de la partida (escritura en caché). Los archivos en `json/` y `mapas/` nunca se modifican durante el juego.

Ver también: [[Mapa]] (usa CacheManager para persistir tiles), [[Enemigos]] (usa DataManager para cargar plantillas), [[Combate]] (guarda héroe tras victoria).

---

## Arquitectura de persistencia

```
json/objetos.json  ──lee──▶  DataManager  ──carga──▶  Jugador, Objetos
json/enemigos.json ──lee──▶  EnemyFactory
mapas/nivel1.txt   ──lee──▶  Mapa

cache/heroe.json   ◀──escribe── CacheManager ◀──recibe── Jugador
cache/mapa_cache.txt ◀──escribe── CacheManager ◀──recibe── Mapa
cache/partida.flag ◀──crea── CacheManager
```

---

## Estructura de archivos

```
proyecto/
├── json/              ← originales (lectura)
│   ├── objetos.json
│   ├── enemigos.json
│   └── heroe.json     ← template default, solo lectura
├── mapas/             ← originales (lectura)
│   ├── nivel1.txt
│   └── nivel2.txt
└── cache/             ← generado en tiempo de juego (escritura)
    ├── heroe.json     ← estado completo del héroe (14 campos + inventario)
    ├── mapa_cache.txt ← mapa con tiles modificados (B/H → '.')
    └── partida.flag   ← flag de existencia (archivo vacío)
```

---

## Formato de archivos JSON

### `json/objetos.json` — Base de datos de objetos

```json
{
    "arma": [
        { "nombre": "Espada Gallo", "descripcion": "...", "dano": 7 },
        { "nombre": "La Espada del Codigo Fuente", "descripcion": "...", "dano": 45 }
    ],
    "pocion": [
        { "nombre": "Alka-Seltzer", "descripcion": "...", "curacion": 12 },
        { "nombre": "Pocion Cubana", "descripcion": "...", "curacion": 200 }
    ],
    "clave": [
        { "nombre": "Aire siniestro", "descripcion": "..." },
        { "nombre": "Tarjeta de Credito Ilimitada", "descripcion": "..." }
    ]
}
```

Contenido actual: 22 armas, 8 pociones, 2 objetos clave.

### `json/heroe.json` — Template del héroe (solo lectura)

```json
{ "nombre": "...", "salud": 100, "ataque": 15, "defensa": 10, "nivel": 1, "pociones": 3, "mana": 50 }
```

### `cache/heroe.json` — Estado completo de la partida

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

### `json/enemigos.json`

Ver [[Enemigos#Formato JSON]] para el formato completo.

---

## CacheManager — API

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

### Cuándo se guarda cada cosa

| Evento | Se guarda en caché |
|--------|-------------------|
| Derrotar jefe (tile 'B' → '.') | `guardarMapa()` |
| Recoger poción (tile 'H' → '.') | `guardarMapa()` |
| Terminar combate (victoria) | `guardarHeroe()` (desde `batalla()`) |
| Salir del juego con 'Q' | `guardarHeroe()` + `guardarMapa()` |

### Formato del mapa en caché

`cache/mapa_cache.txt` es idéntico al original (`vector<string>`), solo con los tiles modificados (B/H → `.`). Usa `Mapa::guardar(archivo)` que serializa línea por línea.

---

## DataManager — Carga de datos

Namespace que centraliza toda la carga/guarda de datos JSON:

| Método | Función |
|--------|---------|
| `cargarObjetos()` | Lee `json/objetos.json` → `map<string, shared_ptr<Objeto>>` |
| `rng()` | Generador `std::mt19937` centralizado |

> `DataManager::cargarHeroe()`/`guardarHeroe()` existen pero son legacy — fueron reemplazados por `CacheManager`. Ver [[Registro/Decisiones#DataManager legacy]].

---

## Config — Paths centralizados

```cpp
namespace Config {
    string objetosPath();     // "json/objetos.json"
    string enemigosPath();    // "json/enemigos.json"
    string heroePath();       // "json/heroe.json" (template)
    string heroeCachePath();  // "cache/heroe.json" (partida)
    string mapaPath(int n);   // "mapas/nivelN.txt"
    string mapaCache();       // "cache/mapa_cache.txt"
}
```

---

## Flujo de carga

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

---

## Método especial: `agregarObjetoSilencioso()`

`Jugador::agregarObjetoSilencioso()` agrega un objeto al inventario sin mostrar el prompt de equipar. Es necesario al cargar una partida desde caché, donde no hay interacción del usuario.

---

## Formato de enemigos.json

Ver [[Enemigos#Formato JSON]] — el formato está documentado en el doc de enemigos.
