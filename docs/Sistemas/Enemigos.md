---
creado: 22/07/2026
modificado: 08/09/2026
tipo: Avance
tags: # deuda-tecnica, idea-loca, bug-critico, bug, refactor
titulo: Enemigos, Factoria y Encuentros
proyecto: "[[LeyendaFacyt]]"
area: Sistemas
estado: En progreso
prioridad: 2
dificultad: Media
version: 1.0.0
---
# Sistema de Enemigos, Factoría y Encuentros

> Archivos: `lib/Enemigo.hpp`, `src/Enemigo.cpp`, `lib/EnemyFactory.hpp`, `src/EnemyFactory.cpp`, `lib/EncounterManager.hpp`, `src/EncounterManager.cpp`

Tres sistemas independientes que se integran en [[Mapa|GameManager]]:

```
json/enemigos.json  ──lee──▶  EnemyFactory  ──crea──▶  Enemigo (instancia)
                                  │                        │
                                  │                   [[Combate|batalla.cpp]]
                                  │                   (combate)
                                  │
EncounterManager  ◀──integra──  [[Mapa|GameManager]]
  (RNG por paso)                  (máquina de estados)
```

Ver también: [[Combate#Sistema de loot]], [[Guardado#Formato de archivos JSON]].

## Configuración por sección, zona y terreno

La tabla de enemigos pertenece a la zona del mapa y no depende del nivel del
jugador. Los archivos `mapas/nivelN.meta` definen listas ponderadas:

```json
{
  "id": "field",
  "terrain": "plain",
  "enemies": [
    {"id": "goblin", "weight": 10},
    {"id": "orco", "weight": 7}
  ],
  "stat_multiplier": 1.0,
  "xp_multiplier": 1.0,
  "boss_id": "zombie_lunes"
}
```

El terreno seguro usa `safe: true` y desactiva los encuentros. Las zonas
rectangulares pueden superponerse; la zona más pequeña tiene prioridad, por lo
que un refugio no obliga a dividir toda la cuadrícula.

Los multiplicadores de estadísticas y XP pertenecen a la zona. Así una
mazmorra o bosque puede ser más peligroso sin escalar artificialmente según el
nivel del héroe.

## XP calculada

La recompensa usa el nivel propio del enemigo, su tier y el multiplicador de
zona. El nivel del jugador no participa:

```text
XP = XP_BASE × nivelEnemigo × tier × multiplicadorZona
```

El campo `exp` antiguo del JSON se mantiene por compatibilidad, pero el
combate utiliza la recompensa calculada. La XP del jugador se limita al
umbral actual y no puede mostrar valores superiores a `expMax`.

---

## 1. Enemigo — Clase entidad

### Herencia

```
Personaje (clase base abstracta)
  └── Enemigo
        ├── id         : string  (clave única, ej. "goblin", "admin_servidor")
        ├── asciiArt[6]: string  (6 líneas de arte ASCII)
        └── botin      : vector<Drop>  (array extensible de objetos+probabilidad)
```

### Constructor

```cpp
Enemigo(id, nombre, salud, ataque, defensa, nivel, asciiArt[6], botin)
```

### Diferencias con la versión anterior

| Antes | Ahora |
|-------|-------|
| `loot1`, `loot2` fijos | `vector<Drop> botin` (N items) |
| Arte generado por keywords en `batalla.cpp` | Arte incrustado en JSON, almacenado en `asciiArt[6]` |
| Sin `id` | `id` para referenciar desde código |

---

## 2. EnemyFactory — Fábrica de enemigos

### Carga (`cargarDesdeJSON`)

```
JSON (por nivel de diseño del enemigo)
  │
  ├─ "1" → [ Goblin, Orco, Slime, Murcielago, Zombie ]
  ├─ "2" → [ Fantasma, Esqueleto, Carlos, Cajero, ... ]
  ├─ "3" → [ Dragon Pubico, Golem, Ogro, Duende, Ciclope, Espectro ]
  └─ "4" → [ El Gran Administrador del Servidor Caido (boss) ]
         │
         ▼
  struct EnemyTemplate {
      id, nombre,
      salud, ataque, defensa, nivel,
      peso,             ← probabilidad relativa
      asciiArt[6],      ← copiado del JSON
      botin,            ← shared_ptr<Objeto> resueltos
      boss              ← bool
  }
         │
         ▼
  map<int, vector<EnemyTemplate>> plantillas;
```

Cada `Drop` en `botin` resuelve el nombre del objeto contra el `map` de objetos cargados por `DataManager::cargarObjetos()`. Si un objeto no existe, lanza `std::runtime_error`.

El nivel del JSON es el nivel propio del enemigo, no el nivel del jugador ni
necesariamente el número del mapa. La metadata decide qué IDs pueden aparecer
en cada zona, por lo que dos zonas de una misma sección pueden tener tablas
completamente distintas.

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

### API

| Método | Comportamiento |
|--------|---------------|
| `crearEnemigo(nivel)` | Fallback compatible: selección ponderada por nivel de diseño |
| `crearEnemigo(entries, statMultiplier, xpMultiplier)` | Selección desde la tabla de una zona y aplicación de modificadores |
| `crearPorId(id)` | Crea el enemigo exacto asignado al `boss_id` de la zona |

---

## 3. EncounterManager — Encuentros aleatorios

### Terrenos y probabilidades

| Terreno | Probabilidad base orientativa | Uso típico |
|---------|-------------------|------------|
| `CAMINO` | 5% | Rutas seguras, pasillos |
| `LLANURA` | 10% | Salas abiertas, terreno neutral |
| `BOSQUE` | 18% | Zonas densas, alta peligrosidad |
| `MAZMORRA` | 14% | Calabozos, subterráneos |
| `SEGURO` | 0% | Spawn, descanso y preparación |

La metadata puede sustituir el nombre lógico del terreno, su color y su
símbolo visual. `SEGURO` fuerza la probabilidad de encuentro a cero y reinicia
la presión de encuentros al entrar en una zona de descanso.

### Fórmula de encuentro

```
base = probBase × multiplicadorMapa × multiplicadorZona
crecimientoMaximo = base × growth_cap
probabilidad = base + crecimiento gradual hasta crecimientoMaximo

si random(0, 99) < resultado → ENCUENTRO!
```

| Parámetro | Valor |
|-----------|-------|
| `grace_steps` | 4 pasos sin encuentros por defecto |
| `growth_cap` | 20% adicional sobre la base por defecto |
| `multiplier` | Ajuste global del mapa |

- Pasos 1-4: no hay encuentro aleatorio.
- Desde el paso 5: la probabilidad crece gradualmente.
- El crecimiento nunca supera `growth_cap`.
- El contador se resetea al ocurrir un encuentro o al entrar en terreno seguro.

### Curación porcentual

El arte y el nombre de cada enemigo se dibujan con el color configurado para
su `tier` en `tier_colors` del metadata del mapa. Los tiles `h`, `H` y `G` son
recursos de mapa de un solo uso. Su porcentaje se
define en la metadata para que cada sección tenga una economía de curación
distinta. La curación se calcula sobre la vida máxima actual, nunca sobre un
valor fijo.

## 8. Ruta de dificultad de cinco niveles

| Nivel | Enemigos normales | Jefe | Modificadores de zona |
|---|---|---|---|
| 1 | Slime, Goblin | Guardián del Tutorial | 0.9 stats / 0.8 XP |
| 2 | Murciélago, Orco, Jabalina | Rey de la Maleza | 1.0 / 1.0 |
| 3 | Duende, Espectro, Cíclope | Ent Ancestral | 1.08 / 1.15 |
| 4 | Esqueleto, Golem, Bruja | Caballero Cebolla | 1.15 / 1.3 |
| 5 | Gárgola, Ogro | Administrador Final | 1.25 / 1.5 |

La progresión de armas sigue la misma cadencia:
`Espada Gallo → Sopladora → Lanza de Zarzas → Mazo de Mineral → Espada del
Codigo Fuente`. Las pociones se reducen a tres tiers (`25`, `50` y `100` HP)
para que la economía de curación sea legible.

### Integración en [[Mapa|GameManager]]

```cpp
// En moverJugador():
if (es transitable) {
    mover jugador;
    handleTile(tile);  // B, K, H tiles first
    
    if (no hubo evento especial) {
        encounterMgr.registrarPaso();
        if (encounterMgr.verificarEncuentro()) {
            iniciarCombate();  // EnemyFactory::crearEnemigo(nivel)
        }
    }
}
```

---

## 4. Formato JSON (`json/enemigos.json`)

### Estructura por nivel

```json
{
  "1": [
    {
      "id": "goblin",
      "nombre": "Goblin",
      "salud": 40,
      "ataque": 8,
      "defensa": 3,
      "peso": 10,
      "ascii": ["     /\\", "    /  \\", "   | <> |", "   | <> |", "   /    \\", "  /______\\"],
      "botin": [
        { "nombre": "Pocion Milagrosa", "prob": 70 },
        { "nombre": "Aire siniestro", "prob": 30 }
      ],
      "boss": false
    }
  ]
}
```

### Campos

| Campo | Tipo | Descripción |
|-------|------|-------------|
| `id` | string | Clave única para referenciar desde código (snake_case) |
| `nombre` | string | Nombre visible en combate |
| `salud` | int | Puntos de vida |
| `ataque` | int | Daño base |
| `defensa` | int | Reducción de daño |
| `peso` | int | Probabilidad relativa de aparición (más alto = más común) |
| `ascii` | string[6] | Arte ASCII de 6 líneas |
| `botin` | array | Lista de drops con nombre del objeto y probabilidad |
| `boss` | bool | `true` si es el jefe del nivel |

### Reglas del botín

- La probabilidad es **acumulativa**: se suman las `prob` de cada entrada
- Se tira `random(0, 99)` y se compara contra el acumulado
- Si la suma total < 100, hay chance de no obtener loot
- El `nombre` debe coincidir exactamente con un objeto en `objetos.json`

### Distribución de pesos por nivel

| Nivel | Enemigos | Rango de pesos |
|-------|----------|----------------|
| 1 | Goblin, Orco, Slime, Murcielago, Zombie | 6-10 |
| 2 | Fantasma, Esqueleto, Carlos, Cajero, Golem, etc. | 3-8 |
| 3 | Dragon, Golem, Ogro, Duende, Ciclope, Espectro | 4-8 |
| 4 | El Gran Administrador (BOSS) | 10 |

---

## 5. Flujo completo de un encuentro aleatorio

```
Jugador presiona W
  │
  ├─ moverJugador(0, -1)
  │   ├─ esTransitable? → Sí (tile = '.')
  │   ├─ jugador.setPos(nuevoX, nuevoY)
  │   ├─ handleTile('.') → nada especial
  │   └─ encounterMgr.registrarPaso()
  │       └─ encounterMgr.verificarEncuentro()
  │           ├─ pasosDesdeUltimo < 4? → No
  │           ├─ calcula base × multiplicadores y crecimiento
  │           ├─ respeta growth_cap de la metadata
  │           └─ 7 < probabilidad → VERDADERO
  │
  └─ iniciarCombate()
      ├─ zonaActual().encounters
      │   └─ enemyFactory.crearEnemigo(tablaPonderada, modificadores)
      │       └─ selecciona un ID de la zona
      └─ [[Combate|batalla(jugador, enemigo)]]
          ├─ Victoria → exp + loot → [[Guardado|guardar héroe]]
          └─ Vuelve al OVERWORLD
```

## 6. Flujo de jefe

```
Jugador pisa tile 'B'
  │
  ├─ handleTile('B')
  │   ├─ zonaActual().boss_id
  │   ├─ enemyFactory.crearPorId(boss_id)
  │   └─ [[Combate|batalla(jugador, jefe)]]
  │       └─ Si victoria → jefeDerrotado = true y se habilita K
  │
  └─ Boss derrotado → tile se marca como '.' (no reaparece)
```

## 7. Archivos involucrados

| Archivo | Rol |
|---------|-----|
| `json/enemigos.json` | Datos de todos los enemigos |
| `json/objetos.json` | Objetos referenciados por `botin` |
| `lib/Enemigo.hpp` / `src/Enemigo.cpp` | Clase entidad enemigo |
| `lib/EnemyFactory.hpp` / `src/EnemyFactory.cpp` | Fábrica: carga, almacena, crea |
| `lib/EncounterManager.hpp` / `src/EncounterManager.cpp` | Gestor de encuentros aleatorios |
| `lib/GameManager.hpp` / `src/GameManager.cpp` | [[Mapa|FSM]] + integración |
| `lib/Batalla.hpp` / `src/batalla.cpp` | [[Combate]] (lee botín y arte) |
| `lib/DataManager.hpp` / `src/DataManager.cpp` | RNG central, carga de objetos |
| `mapas/nivel1.txt` | Mapa con tiles especiales (B, K, H) |
