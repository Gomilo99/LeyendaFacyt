Los combates contra jefes no permiten huir. La opción permanece visible como
`No huir` para mantener estable el layout del menú, pero no ejecuta ninguna
acción de escape. Los enemigos se muestran con el color de su tier configurado
en la metadata del nivel.
---
creado: 22/07/2026
modificado: 23/07/2026
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

Ver también: [[Inventario]] (se integra como acción en combate), [[Guardado]] (se guarda tras cada victoria), [[Plataforma]] (detección de terminal y input).

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
| `ENEMY_TURN` | Turno del enemigo (con pausa de 300ms) |
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
| **[[Inventario]]** | Abre `InventoryUI` con navegación W/S/A/D/SPACE/Q. Overlay sobre el frame de combate |
| **Huir**           | 50% de probabilidad de éxito. Si falla, el enemigo ataca                             |

## Estadísticas del jugador

| Stat | Descripción |
|------|-------------|
| **HP** | Salud actual / máxima. Al llegar a 0 el jugador muere |
| **MP** | Maná para hechizos. Base 50 + 10 por nivel |
| **Ataque** | Daño base. Se incrementa con armas equipadas y al subir de nivel |
| **Defensa** | Reduce el daño recibido. Daño real = `max(1, ataque - defensa)` |

## Sistema de nivelación

- Por batalla ganada: XP = `nivel * 50`
- Al alcanzar la XP necesaria: sube de nivel
- Al subir: 
	- HP_max += SALUD_POR_NIVEL * (nivel+1) -> HP se restaura al máximo, 
	- ataque += ATAQUE_POR_NIVEL * (nivel+1), 
	- defensa += DEFENSA_POR_NIVEL * (nivel+1), 
	- XP_necesaria += EXP_INCREMENTO
- Caso especial: en nivel 3, XP_necesaria se fija en 700

> **Deuda técnica**: Estos valores son magic numbers hardcodeados. Ver [[Registro/Decisiones#Magic Numbers]] para el plan de reemplazo por `constexpr`.

## suppressCout

Durante el combate, las funciones `Jugador::atacar()`, `Jugador::usarMagia()`, `Enemigo::atacar()` y `Personaje::recibirDano()` escriben directamente a `std::cout`. Estos mensajes desincronizarían el ScreenBuffer.

1. `BattleSystem::suppressCout()` redirige `cout.rdbuf()` a un `ostringstream` interno
2. Se ejecuta la acción de combate — los cout se descartan
3. `BattleSystem::restoreCout()` restaura el buffer original

Esto permite que las clases de personaje no necesiten saber si hay un BattleSystem activo.

## Arte ASCII de enemigos

El arte proviene del campo `ascii` (array de 6 strings) en `json/enemigos.json`. Si un enemigo no tiene `ascii` definido, `BattleSystem::generateEnemyArt()` genera arte por keywords:

| Keyword | Arte |
|---------|------|
| `dragon`/`admin` | Dragón/BOSS |
| `golem`/`ogro` | Golem |
| `fantasma`/`espectro` | Fantasma |
| `esqueleto` | Esqueleto |
| `cajero` | Cajero automático |
| `gargola`/`caballero` | Gárgola |
| `ciclope` | Cíclope |
| `slime` | Slime |
| `goblin`/`duende` | Goblin |
| `orco` | Orco |
| `zombie`/`bruja` | Zombie |
| otro | Forma genérica |

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
Batalla.hpp → Enemigo.hpp, Jugador.hpp, CacheManager.hpp
batalla.cpp → ... Inventario.hpp, Platform.hpp, DataManager.hpp, GameBalance.hpp
```
