---
alias:
tags:
  - gamedev
  - JRPG
  - unity
  - estudio
creado: 20/06/2026
modificado: 23/07/2026
estado: En progreso
tipo: Proyecto
base:
  - "[[02-Proyectos]]"
  - "[[A-GameDev]]"
fecha_inicio: 2026-06-19
fecha_fin_prevista: 2026-11-20
progreso: 50%
titulo: Leyenda Facyt
version: 1.1a
---
# Leyenda Facyt
## Descripción
Es un videojuego basado en terminal hecho en C++ que nació como prueba personal para entender la programación orientada a objetos y mejorar mis habilidades de programación. Es un juego de rol donde controlas a un personaje que debe enfrentar a diversos enemigos en un mapa con vista de pájaro (top-down) mientras subes de nivel y obtienes objetos.

En su fase temprana solo se contaba con un archivo que condensaba todas las clases y la lógica del juego, el juego contaba con un sistema de inventario básico, enemigos, un mapa donde moverse y derrotar enemigos. El juego era jugable pero cojo en rendimiento y posibilidades.

Después de un año se retomó el videojuego como prueba de la IA-LLM Big Peackle usando Open Code Desktop, luego de empeoramiento del servicio para estudiantes de Github Copilot.

Actualmente el juego ya cuenta con 
- Soporte multiplataforma entre Windows y Linux.
- Mejor encapsulamiento: separación de clases, hpp sobre cpp, etc.
- Una renovación total en tema de UI y diseño de la interfaz, con movimiento por las opciones de manera mas responsiva, una pantalla principal y menús.
- Una renovación del funcionamiento del inventario que se suma a la de de UI con scrolling y visualización de los datos de las armas y los stats del jugador.
- Colores para los menús y los mapas donde se diferencia entre paredes, enemigos, jefes y puntos de curación.
- Renovación total de los enemigos hacia un enfoque de encuentros aleatorios y un constructor "Enemy Factory".
- Una renovación del menú de combate, 
- Sistema de guardado y carga de partida.
- Mejoras en el gameplay como agregar ataques de maná (que usen maná aún no está implementado) y visualización de un arte del enemigo, barras de vida y log de combate.
### Justificación
El motivo inicial era servir como terreno de pruebas para experimentar, entender mejor la programación orientada a objetos, mejorar la habilidades de programación y diversión.

En este momento este proyecto sirve como medio para entender el desarrollo de videojuegos RPG desde una perspectiva un poco más simple con respecto a hacerlo en Unity, debido a la cantidad de clases preexistentes y que la atención se desvía hacia la integración de arte y otros sistemas "mas de juego" que en la programación de los sistemas básicos.

El problema más grande de este proyecto, al igual que el de Unity es convivir con las tareas de la universidad y, en este semestre (5to) con las tareas incesantes de Sistemas Operativos. Adicionalmente, otro factor de riesgo es el abandono producto de plazos laxos y largas esperas entre sesiones de desarrollo.

---
## Estado del proyecto

| Métrica                   | Valor                              |
| ------------------------- | ---------------------------------- |
| **Objetivos completados** | 2/7 (25%, 40%)                     |
| **Objetivo actual**       | 50% — Multi-nivel y balanceo       |
| **Niveles implementados** | 1 (de 3+ planeados)                |
| **Enemigos**              | 35 (4 niveles de dificultad)       |
| **Objetos**               | 32 (22 armas, 8 pociones, 2 clave) |
| **Plataformas**           | Windows + Linux                    |
| **Lenguaje**              | C++17                              |
| **Build system**          | GNU Make                           |

### Sistemas implementados

| Sistema                                          | Estado     | Doc                     |
| ------------------------------------------------ | ---------- | ----------------------- |
| [[Sistemas/Combate]] por turnos con ScreenBuffer | ✅ Completo | [[Sistemas/Combate]]    |
| [[Sistemas/Inventario]] con UI tipo overlay      | ✅ Completo | [[Sistemas/Inventario]] |
| [[Sistemas/Enemigos]] con factory y encuentros   | ✅ Completo | [[Sistemas/Enemigos]]   |
| [[Sistemas/Mapa]] 2D top-down + FSM              | ✅ 1 nivel  | [[Sistemas/Mapa]]       |
| [[Sistemas/Guardado]] con caché                  | ✅ Completo | [[Sistemas/Guardado]]   |
| [[Sistemas/Plataforma]] multiplataforma          | ✅ Completo | [[Sistemas/Plataforma]] |
#### 🕒 Actividad Reciente
```dataview
LIST 
FROM "02-Proyectos/LeyendaFacyt"
WHERE proyecto = [[LeyendaFacyt]]
SORT file.mday desc
LIMIT 5
```
---
## Navegación del vault
```dataview
TABLE without id link(file.link, titulo) as "Título", 
	"**" + area + "**" as "Área", estado AS "Estado", prioridad AS "Prioridad",
	tag AS "Tags"
FROM "02-Proyectos/LeyendaFacyt"
WHERE proyecto = [[LeyendaFacyt]]
SORT area ASC, prioridad ASC
```
### Sistemas (documentación técnica)
- [[Sistemas/Combate]] — ScreenBuffer, BattleSystem, acciones, stats, nivelación
- [[Sistemas/Inventario]] — InvRenderer, InventoryUI, layout, categorías
- [[Sistemas/Enemigos]] — EnemyFactory, EncounterManager, loot, JSON
- [[Sistemas/Mapa]] — FSM, tiles, flujo de datos, dependencias
- [[Sistemas/Guardado]] — CacheManager, DataManager, formatos JSON, API
- [[Sistemas/Plataforma]] — Platform.hpp, Makefile, compile
### Planificación
- [[Roadmap]] — Objetivos del 25% al 100%, fechas, tareas con Tasks
- [[Sprint]] — Trabajo del sprint actual (julio 2026)
- [[Backlog]] — Ideas y mejoras futuras
### Registro
- [[Cambios]] — Logs técnicos detallados por fecha
- [[Decisiones]] — Por qué se tomaron las decisiones de diseño
### Referencia
- [[Arquitectura]] — Visión general de la arquitectura del sistema
- [[Gemini]] — Prompt original de diseño con IA (referencia histórica)
---
## Tareas del Sprint Actual

```tasks
not done
path includes /Planificacion/Sprint.md
sort by priority
```

## Deuda Técnica Pendiente

```tasks
not done
tag includes #deuda-tecnica
sort by priority
```

---
## Tareas

- [ ] Continuidad entre niveles - nivel2.txt existe pero **el juego siempre carga nivel 1**. No hay transición.
- [ ] Acción Defender - Declarada en el roadmap pero nunca implementada.
- [ ] Armadura equipable - Solo hay slot de arma.
- [ ] Terreno configurable - EncounterManager tiene 4 terrenos pero GameManager nunca llama setTerreno().
---
## Objetivos

### Fase A 40% - Renovación total de sistemas
- [x] Objetivo 25% - Creación y funcionamiento general del videojuego  [completion:: 2026-06-19]
- [x] Objetivo 40% - Renovación y creación de sistemas responsive y sistemas más funcionales.  [completion:: 2026-06-19]

### Fase B 50% - Completar con lo Básico
- [ ] Transición entre niveles (nivel 1→ 2 → 3 → jefe final).
- [ ] 3+ mapas con diseños distintos y temáticas.
- [ ] Configuración terreno por mapa en `EncounterManager`, encuentros aleatorios ajustados por terreno.
- [ ] Reemplazar magic numbers por ``constexpr`` (revisar que significa e implica este cambio).
- [ ] Balancear curva de dificultad entre niveles, escalable.

### Fase C - Refactor (deuda técnica)
- [ ] Extraer UI de `Jugador` (eliminar cout/cin de las clases de modelo).
- [ ] Limpiar `DataManager:cargarHeroe()`/`guardarHeroe()` (ya reemplazado por CacheManager).

### Fase D 75% - Features nuevas
- [ ] Acción Defender en combate.
- [ ] Armadura como equipable.
- [ ] Subjefes y jefe final con lore.
- [ ] Historia básica (texto entre niveles).

### Fase E 85% - Polish 1
- [ ] NPC tienda (usa el sistema de inventario existente).
- [ ] Letra a letra en encuentros con jefes.
- [ ] Colores por rareza de objetos.
- [ ] Playtesting general.
- [ ] Mejora en historia, mensajes predeterminados.
- [ ] Puzzles.

### Fase F 95% - Polish 2
- [ ] Playtesting y balance final.
- [ ] Corrección de bugs.
- [ ] Ajuste de historia, UI, Sistemas y rutas de diseño.

### Fase G 100% - Publicación
- [ ] Arte de portada.
- [ ] README completo con screenshots.
- [ ] Trailer.

### Features Extras
- [ ] Integración de animaciones (CLI) como movimientos de pantalla.
- [ ] Efectos de sonido y música.

**Fecha prevista de entrega**: Finales de 2026

---
## Próximos Pasos
- Principalmente, empieza por establecer rutas de progreso divertidas, alineadas a un balanceo total con posible recorte de los enemigos y objetos. 
- A esto se le puse sumar la creación y conexión de nuevos niveles, delimitación de zonas para la generación de enemigos y posible escalado adicional de niveles y dificultad de enemigos. 
- Se podría integrar escritura letra a letra para el encuentro con jefes, mejorar el log para no que no se pierda la cantidad de daño hecho al enemigo.
- También se podría integrar nuevos mini jefes con algunos objetos nuevos.
- Tal ves un sistema mejorado para los objetos con rarezas: objetos más raros tienen colores diferentes.
- Posible reconstrucción de las bases de datos de los objetos y enemigos para facilitar el balanceo y la construcción de nuevos registros.

---
## Workflow de desarrollo
1. **Abrir Sprint** → Ver [[Sprint]] para tareas planificadas
2. **Desarrollar** → Si se toma una decisión, documentar en [[Decisiones]]
3. **Al terminar** → Marcar tarea `[x]` en [[Sprint]], crear entrada en [[Cambios]]
4. **Commit** → `git commit` con mensaje descriptivo
5. **Review semanal** → Actualizar [[Roadmap]], mover tareas vencidas
### Convenciones de archivos

| Situación | Acción |
|-----------|--------|
| Nuevo sistema de juego | Crear `docs/Sistemas/NuevoSistema.md` con tag `#sistema/nuevo` |
| Decisión de diseño | Agregar en [[Decisiones]] |
| Bug arreglado | Agregar entrada en [[Cambios]] |
| Idea futura | Agregar en [[Backlog]] con tag `#plan/backlog` |
| Feature planificada | Agregar en [[Sprint]] con tag `#plan/sprint` |

### Tags

| Tag                   | Uso                                      |
| --------------------- | ---------------------------------------- |
| `#sistema/combate`    | Sistema de combate                       |
| `#sistema/inventario` | Sistema de inventario                    |
| `#sistema/enemigos`   | Sistema de enemigos                      |
| `#sistema/mapa`       | Sistema de mapa y FSM                    |
| `#sistema/guardado`   | Sistema de guardado                      |
| `#sistema/plataforma` | Capa multiplataforma                     |
| `#registro/cambio`    | Log de cambios                           |
| `#registro/decisión`  | Decisión de diseño                       |
| `#plan/roadmap`       | Elemento del roadmap                     |
| `#plan/sprint`        | Tarea del sprint actual                  |
| `#plan/backlog`       | Idea o mejora futura                     |
| `#deuda-tecnica`      | Deuda técnica conocida                   |
| `#facyt/bug`          | Algo que rompe el juego                  |
| `#facyt/refactor`     | Código que funciona pero es feo          |
| `#idea`               | Idea nueva sin desarrollar para el juego |
### Gestión de Prioridades
#### 🔴 Prioridad 1: Bloqueadores y Núcleo (MVP - Producto Mínimo Viable)

**Criterio:** Si este archivo/sistema no funciona o no está definido, el juego **no se puede jugar** o el desarrollo está detenido.

- **Sistemas Críticos:** El `GameManager`, la lógica de colisiones, el sistema de turnos básico.
- **Bugs Fatales:** Errores de memoria (segmentation faults) o fallos en el `ScreenBuffer`.
- **Documentación:** La [Arquitectura](obsidian://open?file=02-Proyectos%2FLeyendaFacyt%2FArquitectura.md) entra aquí, porque si no sabes cómo se conectan las clases, escribirás código que luego tendrás que borrar.
- _Ejemplo actual:_ Implementar la transición de niveles (estás al 50%, sin esto no hay juego completo).

#### 🟡 Prioridad 2: Funcionalidad y Estabilidad (Experiencia de Juego)

**Criterio:** El juego funciona, pero le falta "carne" o el código es difícil de mantener (deuda técnica).

- **Sistemas de Soporte:** El sistema de [Inventario](obsidian://open?file=02-Proyectos%2FLeyendaFacyt%2FSistemas%2FInventario.md), el balanceo de enemigos, nuevos tipos de objetos.
- **Refactorización Importante:** Extraer la UI de la clase `Jugador` (lo tienes en tu lista de tareas). No impide jugar, pero te hará la vida imposible después si no lo haces.
- **Contenido Base:** Crear los mapas de los niveles 2 y 3.
- _Ejemplo actual:_ Reemplazar los _magic numbers_ por `constexpr`.

#### 🔵 Prioridad 3: Pulido y Estética (Polish)

**Criterio:** Cosas que hacen que el juego se vea "profesional" o sea más divertido, pero que no afectan la lógica subyacente.

- **Visuales:** Mejorar el arte ASCII de los enemigos, efectos de "letra a letra" en los textos.
- **Lore/Historia:** Escribir los diálogos de los NPCs o la historia de fondo.
- **Features Extra:** El sistema de "Puzzles" o colores por rareza de objetos.
- _Ejemplo actual:_ El sistema de sonidos (si llegaras a implementarlo) o mensajes de ambiente.