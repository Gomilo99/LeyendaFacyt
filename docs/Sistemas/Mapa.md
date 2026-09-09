---
creado: 22/07/2026
modificado: 08/09/2026
proyecto: "[[LeyendaFacyt]]"
area: Sistemas
estado: En Progreso
prioridad: 2
tags:
tipo: Avance
titulo: Mapa y FSM 
version: 1.0.0 # La version inicial es 0.1
dificultad: Media # Muy Baja, Baja, Media, Alta, Muy Alto
---
## Links
- Documento Gestor - [[LeyendaFacyt]]
- Sistema de Mapa - [[Mapa]]
# Sistema de Mapa y FSM del Juego

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
| `h` | Poción pequeña (25% de vida máxima) |
| `H` | Poción mediana (50% de vida máxima) |
| `G` | Poción grande (100% de vida máxima) |
| `,` | Pradera decorativa, transitable |
| `;` | Bosque o hierba, transitable |
| `~` | Agua u océano, transitable con semántica configurable |
| `d` | Sala de mazmorra, transitable |
| `f` | Piso de mazmorra final, transitable |
| `s` | Zona segura, transitable y sin encuentros |

Los caracteres de terreno son parte de la geometría visual del `.txt`, no
coordenadas codificadas en la metadata. Esto permite ampliar una zona
dibujando nuevas celdas con el mismo símbolo. Las reglas de enemigos y
multiplicadores se asocian a ese único símbolo mediante `tile` en `.meta`.
Por ejemplo, `"tile": "."` es el piso general, `","` la pradera, `";"` el
bosque, `"d"` una mazmorra y `"s"` una zona segura.

El renderizador duplica horizontalmente cada celda (`símbolo + espacio`) para
compensar la relación de aspecto habitual de las terminales. La representación
del muro es ahora explícita: el mapa puede usar `-`, `|`, `+` y `=` como
tiles de pared y el renderizador los imprime directamente, sin inferir
esquinas ni consultar vecinos. `#` se conserva únicamente como compatibilidad
con mapas antiguos.

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

La campaña contiene cinco mapas jugables, cargados en este orden:

| Nivel | Rol | Límite | Identidad |
|---|---|---:|---|
| 1 | Tutorial | 1 | Recorrido corto y jefe de aprendizaje |
| 2 | Pradera | 2 | Primer mapa abierto y primera mejora |
| 3 | Bosque | 3 | Mayor presión y enemigos de dos tiers |
| 4 | Mazmorra 1 | 4 | Pasillos, defensas altas y arma pesada |
| 5 | Mazmorra final | 5 | Recursos limitados y jefe de campaña |

Cada mapa separa geometría (`nivelN.txt`) y reglas (`nivelN.meta`). Las zonas
se amplían directamente dibujando más celdas con su símbolo; la zona segura se
delimita con `s` alrededor del spawn sin coordenadas adicionales.

| Archivo | Dimensiones | Descripción |
|---------|-------------|-------------|
| `mapas/nivel1.txt` | 30x12 | Tutorial con piso `.` |
| `mapas/nivel2.txt` | 36x13 | Pradera con terreno `,` |
| `mapas/nivel3.txt` | 40x15 | Bosque con terreno `;` |
| `mapas/nivel4.txt` | 44x16 | Mazmorra con terreno `d` |
| `mapas/nivel5.txt` | 48x18 | Mazmorra final con terreno `f` |

Cada mapa tiene un archivo lateral `nivelN.meta`. Cada zona declara un único
`tile`; no existen rectángulos, rangos ni prioridades geométricas.

---

## Tiles especiales — Flujo de eventos

```
OVERWORLD
  │
  ├─ WASD ──────────→ mover jugador
  │                     │
  │                     ├─ tile 'B' ──→ jefe de la zona (`boss_id`) → [[Combate|batalla()]]
  │                     │               ├─ victoria → mapa.setTile('.') → [[Guardado|guardarMapa()]]
  │                     │               └─ derrota  → GAME_OVER
  │                     ├─ tile 'K' ──→ siguiente sección o victoria final
  │                     ├─ tile 'h/H/G' ──→ curación porcentual → tile → '.'
  │                     └─ tile '.' ──→ [[Enemigos|EncounterManager::checkEncounter()]]
  │                                       │
  │                                       ├─ true  → enemigo de la zona → [[Combate|batalla()]]
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
        ├── tile 'h/H/G' → curación porcentual → setTile('.')
        ├── tile 'K' → siguiente sección o victoria final
        └── 'Q' → [[Guardado|guardar]] → salir
```

---

## Dependencias entre archivos

```
GameManager   → DataManager, CacheManager, batalla.hpp, mapa.hpp,
                jugador.hpp, enemyFactory.hpp, encounterManager.hpp,
                MapMetadata.hpp
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

## Metadatos de mapas

Cada `mapas/nivelN.meta` acompaña al mapa y permite cambiar el balance sin
rediseñar la cuadrícula. Define sección, límite de nivel, probabilidad base,
multiplicador del mapa, pasos de gracia, crecimiento máximo, estilos de terreno,
curación y zonas rectangulares.

Una zona puede ser segura, modificar estadísticas/XP, listar enemigos por peso
y asignar el jefe exacto del tile `B` mediante `boss_id`. Las zonas pequeñas
superpuestas tienen prioridad para evitar fragmentar el mapa en muchas regiones.

El HUD muestra sección, zona, terreno y límite. `8` o `F8` alterna el límite
para depuración. La XP nunca supera el umbral actual.
# Metadatos de mapas

Cada `mapas/nivelN.meta` acompaña al mapa y permite cambiar balance sin
rediseñar la cuadrícula. Define el límite de nivel de la sección, multiplicador
de encuentros y crecimiento máximo (hasta 20%), colores/estilos de terreno,
curación de `h`/`H`/`G` y zonas rectangulares. Una zona puede ser segura,
modificar estadísticas/XP y listar sus enemigos por peso; `boss_id` selecciona
exactamente el jefe del tile `B`. El HUD muestra sección, terreno, zona y
límite. `8` (o F8 en Windows) alterna el límite para depuración.
