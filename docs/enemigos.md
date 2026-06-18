# Sistema de Enemigos, Factoría y Encuentros

## Arquitectura general

```
json/enemigos.json  ──lee──▶  EnemyFactory  ──crea──▶  Enemigo (instancia)
                                  │                        │
                                  │                   batalla.cpp
                                  │                   (combate)
                                  │
EncounterManager  ◀──integra──  GameManager
  (RNG por paso)                  (máquina de estados)
```

Tres sistemas independientes que se integran en `GameManager`:

---

## 1. `Enemigo` — Clase entidad (`lib/Enemigo.hpp`, `src/Enemigo.cpp`)

### Responsabilidad
Representa una instancia concreta de enemigo **lista para el combate**: con sus stats actuales, arte ASCII y botín.

### Herencia
```
Personaje (clase base abstracta)
  └── Enemigo
        ├── id         : string  (clave única, ej. "goblin", "admin_servidor")
        ├── asciiArt[6]: string  (6 líneas de arte ASCII)
        └── botin      : vector<Drop>  (array extensible de objetos+probabilidad)
```

### Diferencias con la versión anterior
| Antes | Ahora |
|-------|-------|
| `loot1`, `loot2` fijos | `vector<Drop> botin` (N items) |
| Arte generado por keywords en `batalla.cpp` | Arte incrustado en JSON, almacenado en `asciiArt[6]` |
| Sin `id` | `id` para referenciar desde código |

### Constructor
```cpp
Enemigo(id, nombre, salud, ataque, defensa, nivel, asciiArt[6], botin)
```

---

## 2. `EnemyFactory` — Fábrica de enemigos (`lib/EnemyFactory.hpp`, `src/EnemyFactory.cpp`)

### Responsabilidad
Lee el archivo JSON, almacena las plantillas organizadas por nivel, y crea instancias de `Enemigo` listas para combatir.

### Flujo interno

#### 2.1 Carga (`cargarDesdeJSON`)
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

El `map` tiene como clave el nivel entero (`1`, `2`, `3`, `4`) y como valor un vector con todas las plantillas de ese nivel.

Cada `Drop` en `botin` resuelve el nombre del objeto contra el `map` de objetos cargados previamente por `DataManager::cargarObjetos()`. Si un objeto no existe, lanza `std::runtime_error`.

#### 2.2 Selección ponderada (`seleccionarPlantilla`)

```
Input: nivel (ej. 2)
  │
  ├─ Busca plantillas[nivel] = vector de 12 enemigos
  ├─ Suma todos los peso → totalPeso (ej. 60)
  ├─ uniform_int_distribution(0, totalPeso - 1) → tirada (ej. 42)
  │
  └─ Itera enemigos restando peso:
       Fantasma (peso 8)   → tirada 42 - 8  = 34
       Fantasma Visto (6)  → 34 - 6  = 28
       Esqueleto (7)       → 28 - 7  = 21
       Carlos (5)          → 21 - 5  = 16
       Golem Plastilina (6)→ 16 - 6  = 10
       Golem Facturas (5)  → 10 - 5  = 5
       Bruja (4)           → 5 - 4   = 1
       Cajero (3)          → 1 - 3   = -2  ← NEGATIVO: seleccionado!
                                │
                                ▼
                         Se devuelve "Cajero Automatico"
```

Este método asegura que un enemigo con `peso` = 10 aparece el doble de veces que uno con `peso` = 5.

#### 2.3 Creación de instancias

| Método | Comportamiento |
|--------|---------------|
| `crearEnemigo(nivel)` | Selección ponderada, devuelve `Enemigo` instanciado |
| `crearJefe(nivel)` | Busca desde `nivel` hacia abajo el primer `boss: true`. Si no encuentra, lanza excepción |
| `hayJefe(nivel)` | `true` si existe algún `boss: true` entre nivel 1 y `nivel` |

### Manejo de errores
- Si un nivel no tiene enemigos: `std::runtime_error`
- Si un objeto del botín no existe en `objetos.json`: `std::runtime_error`
- Si `crearJefe` no encuentra jefe: `std::runtime_error`

---

## 3. `EncounterManager` — Gestor de encuentros aleatorios (`lib/EncounterManager.hpp`, `src/EncounterManager.cpp`)

### Responsabilidad
Decide si al moverse el jugador debe iniciarse un combate, usando RNG con probabilidad variable según el terreno.

### Terrenos y probabilidades

| Terreno | Probabilidad base | Uso típico |
|---------|-------------------|------------|
| `CAMINO` | 5% | Rutas seguras, pasillos |
| `LLANURA` | 10% | Salas abiertas, terreno neutral |
| `BOSQUE` | 18% | Zonas densas, alta peligrosidad |
| `MAZMORRA` | 14% | Calabozos, subterráneos |

### Fórmula de encuentro

```
probabilidad = probBase + (pasosDesdeUltimo - 3) * 3
resultado = clamp(probabilidad, 0, 40)

si random(0, 99) < resultado → ENCUENTRO!
```

- **3 pasos mínimos** de gracia tras cada combate (no hay encuentros)
- A partir del paso 4, la probabilidad **aumenta +3% por paso extra**
- **Tope máximo** de 40% para evitar rachas infinitas
- Al ocurrir un encuentro, el contador se **resetea a 0**

### Integración en GameManager

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

Los tiles especiales (`B`, `K`, `H`) tienen prioridad y evitan el chequeo de encuentro aleatorio.

### Estado interno

| Campo | Descripción |
|-------|-------------|
| `terrenoActual` | Terreno donde se mueve el jugador (afecta prob. base) |
| `pasosDesdeUltimo` | Contador de pasos desde el último encuentro |

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
      "ascii": [
        "     /\\",
        "    /  \\",
        "   | <> |",
        "   | <> |",
        "   /    \\",
        "  /______\\"
      ],
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

## 5. Máquina de estados del juego (`GameManager`)

```
           ┌──────────────────────────────────────────┐
           │           MAIN_MENU                      │
           │  "Presiona Enter para comenzar..."       │
           └─────────────────┬────────────────────────┘
                             │ Enter
                             ▼
           ┌──────────────────────────────────────────┐
           │           OVERWORLD                      │
           │  ┌─ WASD → moverJugador() ────┐          │
           │  │   ├─ tile 'B' → jefe       │          │
           │  │   ├─ tile 'K' → victoria   │          │
           │  │   ├─ tile 'H' → pocion     │          │
           │  │   └─ else → ¿encuentro? ──┤│          │
           │  │                    │       ││          │
           │  │                    ▼       ││          │
           │  │              ┌─ BATTLE ────┘│          │
           │  │              │ victoria/huir│          │
           │  │              └──────┬───────┘          │
           │  │                     │ (vuelve)         │
           │  │                     ▼                  │
           │  ├─ I → mostrarInventario()              │
           │  └─ Q → salir                            │
           └─────────────────┬────────────────────────┘
                             │ jugador muere
                             ▼
           ┌──────────────────────────────────────────┐
           │           GAME_OVER                      │
           │  "Has muerto"                            │
           └──────────────────────────────────────────┘
```

---

## 6. Flujo completo de un encuentro aleatorio

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
      │   ├─ seleccionarPlantilla(1)
      │   ├─ totalPeso = 10+8+7+9+6 = 40
      │   ├─ tirada = random(0,39) = 22
      │   ├─ Goblin(10) → 12, Orco(8) → 4, Slime(7) → -3
      │   └─ Seleccionado: "Slime de Cafe Vencido"
      │
      ├─ Enemigo enemigo = ... (instancia con stats, arte, botín)
      └─ batalla(jugador, enemigo)
          ├─ Combate por turnos
          ├─ Victoria → exp + loot, guardar héroe
          └─ Vuelve al OVERWORLD
```

---

## 7. Integración de jefe

```
Jugador pisa tile 'B' (ej. posición 10,7 en nivel1.txt)
  │
  ├─ handleTile('B')
  │   ├─ iniciarCombateJefe()
  │   │   ├─ enemyFactory.hayJefe(1)? → No (el boss está en nivel 4)
  │   │   ├─ Mensaje: "Aún no hay jefe para tu nivel..."
  │   │   └─ iniciarCombate() normal (aleatorio)
  │   │
  │   └─ Si el jugador está en nivel 4:
  │       ├─ enemyFactory.crearJefe(4)
  │       │   └─ Busca desde nivel 4 hacia abajo:
  │       │       nivel 4 → "El Gran Administrador..." (boss: true)
  │       │       └─ Devuelve Enemigo con 500 HP, 40 ATK, 25 DEF
  │       └─ batalla(jugador, jefe)
  │           └─ Si victoria y nivel ≥ 4 → haGanado = true
  │
  └─ Boss derrotado → tile se marca como '.' (no reaparece)
```

---

## 8. Archivos involucrados

| Archivo | Rol |
|---------|-----|
| `json/enemigos.json` | Datos de todos los enemigos (formato nuevo) |
| `json/objetos.json` | Objetos referenciados por `botin` |
| `lib/Enemigo.hpp` / `src/Enemigo.cpp` | Clase entidad enemigo |
| `lib/EnemyFactory.hpp` / `src/EnemyFactory.cpp` | Fábrica: carga, almacena, crea |
| `lib/EncounterManager.hpp` / `src/EncounterManager.cpp` | Gestor de encuentros aleatorios |
| `lib/GameManager.hpp` / `src/GameManager.cpp` | Máquina de estados + integración |
| `lib/Batalla.hpp` / `src/batalla.cpp` | Combate (lee botín y arte del enemigo) |
| `lib/DataManager.hpp` / `src/DataManager.cpp` | RNG central, carga de objetos y héroe |
| `mapas/nivel1.txt` | Mapa con tiles especiales (B, K, H, E) |
