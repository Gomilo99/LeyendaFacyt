Los combates contra jefes no permiten huir. La opción permanece visible como
`No huir` para mantener estable el layout del menú, pero no ejecuta ninguna
acción de escape. Los enemigos se muestran con el color de su tier configurado
en la metadata del nivel.
---
creado: 22/07/2026
modificado: 14/09/2026
tipo: Avance
tags:
titulo: Combate
proyecto: "[[LeyendaFacyt]]"
area: Sistemas
estado: Completo
prioridad: 1
dificultad: Alta
version: 1.0.0
---
## Links
- Documento Gestor - [[LeyendaFacyt]]
- Sistema de Mapa - [[Mapa]]
- Sistema de Combate - [[Combate]]
# Sistema de Combate 
> Archivos: `lib/Batalla.hpp`, `src/batalla.cpp`

El combate es por turnos con interfaz gráfica ASCII en tiempo real. Usa un `ScreenBuffer` de doble capa con redibujado diferencial y colores ANSI.

## Arquitectura

```
ScreenBuffer ← Renderer ← BattleSystem
                  ↑            ↑
             InputHandler   Jugador / [[Enemigos|Enemigo]]
```

Ver también: [[Inventario]] (se integra como acción en combate, auditoría #19), [[Guardado]] (se guarda tras cada victoria), [[Plataforma]] (detección de terminal, input y limpieza de pantalla/buffer, #20).

## Clases

### ScreenBuffer

Buffer de doble capa (`char[22][56]` + atributos ANSI) con redibujado diferencial: solo las líneas modificadas se reescriben a terminal.

| Método | Función |
|--------|---------|
| `setChar(x, y, char)` | Escribe un carácter |
| `setAttr(x, y, attr)` | Asigna atributo de color ANSI |
| `drawString(x, y, str)` | Escribe una cadena |
| `drawHLine(x, y, len, char)` | Línea horizontal |
| `drawVLine(x, y, len, char)` | Línea vertical |
| `drawBox(x, y, w, h)` | Dibuja un recuadro |
| `drawBar(x, y, w, ratio)` | Barra de progreso (vida/maná) |
| `render()` | Vuelca el buffer a terminal con ANSI |
| `forceRedraw()` | Invalida el caché (redibuja todo en el siguiente frame) |

### Renderer

Compone cada frame del combate sobre un `ScreenBuffer`:

| Sección | Contenido |
|---------|-----------|
| `drawBackground()` | Encabezado "LEYENDA DEL CAMPUS - COMBATE" |
| `drawEnemy()` | Recuadro con nombre + arte ASCII (6 líneas) |
| `drawEnemyHealthBar()` | Barra de HP con color según % (verde/amarillo/rojo) |
| `drawCombatMenu()` | Menú de 4 opciones con selector `>` |
| `drawPlayerInfo()` | Panel: nombre, HP bar, MP bar, valores numéricos |
| `drawLog()` | Mensaje de estado en la última línea |

### InputHandler

Navegación circular W/S por el menú de 4 opciones. Selección con SPACE.

### BattleSystem

Máquina de estados que orquesta el combate completo.

**Estados:**

| Estado | Descripción |
|--------|-------------|
| `PLAYER_TURN` | Esperando entrada del jugador (W/S/SPACE) |
| `PLAYER_ACTION` | Ejecutando la acción seleccionada |
| `ENEMY_TURN` | Turno del enemigo: `Enemigo::ejecutarComportamiento()` (Strategy #12) con pausa de 400ms |
| `ANIMATION` | Reservado para animaciones futuras |
| `VICTORY` | Enemigo derrotado |
| `DEFEAT` | Jugador sin HP |
| `FLEE` | Jugador huyó del combate |

**Flujo:**
```
PLAYER_TURN → PLAYER_ACTION → ENEMY_TURN → PLAYER_TURN (loop)
                                        → VICTORY | DEFEAT | FLEE (terminales)
```

## Acciones en combate
Variables constantes iniciales BASE para los cálculos (entendiendo que podrían variar en un futuro dentro de la propia partida) [[Balance#Constantes de Balanceo]]

| Opción             | Descripción                                                                          |
| ------------------ | ------------------------------------------------------------------------------------ |
| **Atacar**         | Ataque físico: `jugador.atacar(enemigo)` usando ataque base + daño del arma equipada |
| **Magia**          | Hechizo que cuesta 10 MP. Daño = `ataque * 2 + nivel * 5`. Requiere mínimo 10 MP     |
| **[[Inventario]]** | Abre `invUI` (instancia `unique_ptr` reutilizable de `BattleSystem`; auditoría #19) con navegación W/S/A/D/SPACE/Q. Overlay sobre el frame de combate |
| **Huir**           | 50% de probabilidad de éxito. Si falla, el enemigo ataca                             |

## Estadísticas del jugador

| Stat | Descripción |
|------|-------------|
| **HP** | Salud actual / máxima. Al llegar a 0 el jugador muere |
| **MP** | Maná para hechizos. Base 50 + 10 por nivel |
| **Ataque** | Daño base. Se incrementa con armas equipadas y al subir de nivel |
| **Defensa** | Reduce el daño recibido. Daño real = `max(1, ataque - defensa)` |

## Sistema de nivelación

- La XP total de cada batalla la calcula el enemigo vía
  `Enemigo::experienciaCalculada()`: `exp_base × factor_nivel × tier`
  (ver [[Enemigos#8. Ruta de dificultad de cinco niveles]], auditoría #13).
- `Jugador::obtenerExperiencia()` acumula la XP; al superar el umbral actual
  sube de nivel.
- La XP requerida por nivel sale de la tabla `EXP_TABLE[]` de `GameBalance.hpp`
  (curva de costes por nivel, auditoría #15), no de una fórmula cuadrática.
- Al subir de nivel los incrementos son fijos (`SALUD_POR_NIVEL`,
  `ATAQUE_POR_NIVEL`, `DEFENSA_POR_NIVEL`), el HP se restaura al máximo y el
  costo del siguiente nivel avanza según `EXP_TABLE` (auditoría #14).

## Escritura a `std::cout` durante las acciones

Tras la refactorización de acciones (auditoría #3) ya no existe
`suppressCout()`/`restoreCout()`. `Jugador::atacar()` y `Jugador::usarMagia()`
retornan `ActionResult`, y `Personaje` envía sus mensajes a una abstracción
`Output` (por defecto `NullOutput` dentro del combate). Ningún `cout` directo
desincroniza el `ScreenBuffer`.

## Turno del enemigo y comportamiento (auditoría #12)

`BattleSystem::doEnemyTurn()` delega la decisión en `Enemigo::ejecutarComportamiento()`,
que invoca la estrategia `EnemyBehavior` asignada por `EnemyFactory` desde el
JSON (campo `behavior`). La curación se aplica con `recibirDano(-cantidad)` y
descarta el ataque del turno cuando procede.

| Estrategia | Condición de curación | Importe |
|-----------|----------------------|---------|
| `AgresivoBehavior` | nunca | — (siempre ataca) |
| `DefensivoBehavior` | HP < 50% del máximo | 12.5% de HP máximo |
| `SanadorBehavior` | HP < HP máximo | 10% de HP máximo |

Ver [[Enemigos#Comportamiento de combate (patrón Strategy, auditoría #12)]] para
reglas, JSON y wiring en `EnemyFactory`.

## Arte ASCII de enemigos

El arte proviene del campo `ascii` (array de 6 strings) de `json/enemigos.json`,
se almacena en `Enemigo::asciiArt[]` al instanciar desde `EnemyFactory` y
`Renderer::drawEnemy()` lo dibuja con el color del `tier`. Ya no existe
`generateEnemyArt()`: el arte viaja con la instancia y el fallback por keywords
quedó obsoleto. Ver [[Enemigos#4. Formato JSON (json/enemigos.json)]] para el campo.

## Sistema de loot

Cada enemigo tiene un vector de `Drop` (objeto + probabilidad). Al derrotarlo:
- Se itera el vector con probabilidad acumulada
- Si la suma total < 100, hay chance de no obtener loot
- Si es un arma, pregunta si desea equiparla

Ver [[Enemigos#Formato JSON]] para el formato del botín.

## Controles

| Tecla | Acción |
|-------|--------|
| **W/S** | Navegar opciones del menú |
| **SPACE** | Confirmar opción seleccionada |

## Dependencias

```
Batalla.hpp  → Enemigo.hpp, Jugador.hpp (forward de InventoryUI: el include de
               Inventario.hpp va solo en batalla.cpp, #19)
batalla.cpp  → Batalla.hpp, Inventario.hpp (#19), Platform.hpp (clearScreen, #20),
               DataManager.hpp, CacheManager.hpp, GameBalance.hpp
```
