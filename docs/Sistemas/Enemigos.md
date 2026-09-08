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
JSON (por nivel)
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
| `crearEnemigo(nivel)` | Selección ponderada, devuelve `Enemigo` instanciado |
| `crearPorId(id)` | Crea el enemigo exacto asignado al `boss_id` de la zona |
| `hayJefe(nivel)` | `true` si existe algún `boss: true` entre nivel 1 y `nivel` |

---

## 3. EncounterManager — Encuentros aleatorios

### Terrenos y probabilidades

| Terreno | Probabilidad base | Uso típico |
|---------|-------------------|------------|
| `CAMINO` | 5% | Rutas seguras, pasillos |
| `LLANURA` | 10% | Salas abiertas, terreno neutral |
| `BOSQUE` | 18% | Zonas densas, alta peligrosidad |
| `MAZMORRA` | 14% | Calabozos, subterráneos |

> **Nota**: El terreno actual nunca se configura desde `GameManager`. Siempre queda en el valor por defecto. Ver [[Registro/Decisiones#Terreno no configurado]].

### Fórmula de encuentro

```
probabilidad = probBase + max(0, pasosDesdeUltimo - GRACE_PERIOD) * INCREMENTO_POR_PASO
probabilidad = min(probabilidad, CAP)

si random(0, 99) < resultado → ENCUENTRO!
```

| Parámetro | Valor |
|-----------|-------|
| GRACE_PERIOD | 3 pasos sin encuentros |
| INCREMENTO_POR_PASO | +3% por paso extra |
| CAP | 40% máximo |

- Pasos 1-3: probabilidad = baseTerreno (sin encuentros forzados)
- Paso 4: +3%, Paso 5: +6%, ... hasta cap 40%
- El contador se resetea al ocurrir un encuentro
- Tiles especiales (B, K, H) skipean el chequeo de encuentro aleatorio

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
  │           ├─ pasosDesdeUltimo < 3? → No
  │           ├─ prob = 10 + (4-3)*3 = 13
  │           ├─ random(0,99) = 7
  │           └─ 7 < 13 → VERDADERO
  │
  └─ iniciarCombate()
      ├─ enemyFactory.crearEnemigo(jugador.getNivel())
      │   └─ seleccionarPlantilla(1) → "Slime de Cafe Vencido"
      └─ [[Combate|batalla(jugador, enemigo)]]
          ├─ Victoria → exp + loot → [[Guardado|guardar héroe]]
          └─ Vuelve al OVERWORLD
```

## 6. Flujo de jefe

```
Jugador pisa tile 'B'
  │
  ├─ handleTile('B')
  │   ├─ iniciarCombateJefe()
  │   │   ├─ enemyFactory.hayJefe(1)? → No (boss está en nivel 4)
  │   │   └─ iniciarCombate() normal (aleatorio)
  │   │
  │   └─ Si el jugador está en nivel 4:
  │       ├─ enemyFactory.crearJefe(4) → "El Gran Administrador..."
  │       └─ [[Combate|batalla(jugador, jefe)]]
  │           └─ Si victoria → haGanado = true
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
