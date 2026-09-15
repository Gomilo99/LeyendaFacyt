---
creado: 22/07/2026
modificado: 15/09/2026
tipo: Avance
tags: # deuda-tecnica, idea-loca, bug-critico, bug, refactor
titulo: Sprint
proyecto: "[[LeyendaFacyt]]"
area: Planificacion
estado: En progreso
prioridad: 0
dificultad: Media
version: 1.0.0
---
# Sprint Actual

Trabajo planificado para el sprint actual. Al terminar una tarea, marcar como `[x]` y agregar entrada en [[Cambios]].

> **Sprint**: Septiembre 2026 — Refactorización de Arquitectura y Limpieza de Deuda Técnica
> **Objetivo**: Ejecutar los 4 Sprints de la Auditoría de Sistemas (P1 a P4)

---

## Completado este sprint
_(al terminar una tarea, moverla aquí)_
### Auditoria Sistemas Gemini 13-09-2026
[[auditoria_sistemas_13_09_26]]
#### Sprint 1 — Cortar la hemorragia (P1 - Urgente)
- [x] #1 Extraer lógica de tiles y nivelación de `GameManager` hacia `TileHandler` y controladores específicos 📅 2026-09-15 ⏫ #arquitectura #deuda-tecnica [completion:: 2026-09-13]
- [x] #2 Reemplazar `handleTile()` hardcoded por registro `TileEvent` (`map<char, TileHandler>`) 📅 2026-09-15 ⏫ #arquitectura #sistema/mapa [completion:: 2026-09-13]
- [x] #3 Eliminar `suppressCout()`/`restoreCout()` refactorizando `atacar()` y `usarMagia()` para retornar `ActionResult` 📅 2026-09-15 ⏫ #refactor #sistema/combate [completion:: 2026-09-13]
- [x] #5 Eliminar `cin` de `Jugador::agregarObjeto()`, delegando la decisión de equipar a la UI 📅 2026-09-15 ⏫ #refactor #sistema/inventario [completion:: 2026-09-13]
- [x] #9 Agregar versión de formato de guardado (`j["version"] = 1`) y usar `.value()` en `CacheManager` 📅 2026-09-15 ⏫ #sistema/guardado [completion:: 2026-09-13]

#### Sprint 2 — Ordenar la casa (P2 - Importante)
- [x] #4 Refactorizar `batalla()` para retornar `BattleResult` y mover post-combate a `BattleSystem` 📅 2026-09-20 🔺 #refactor #sistema/combate [completion:: 2026-09-13]
- [x] #6 Hacer que `Jugador::obtenerExperiencia()` retorne `LevelUpResult` sin `cout` directo 📅 2026-09-20 🔺 #refactor #sistema/combate [completion:: 2026-09-13]
- [x] #10 Guardar/cargar `saludMaxima` y `manaMaxima` explícitamente en `CacheManager` 📅 2026-09-20 🔺 #sistema/guardado [completion:: 2026-09-13]
- [x] #11 Centralizar guardado en `SaveController` / `CacheManager::guardarPartida` 📅 2026-09-20 🔺 #sistema/guardado [completion:: 2026-09-13]
- [x] #17 Mover configuración de terreno a `EncounterManager::configurarPorNivel` 📅 2026-09-20 🔺 #sistema/mapa [completion:: 2026-09-13]

#### Sprint 3 — Pulir balance y UX (P3 - Mejora)
- [x] #7 Eliminar función muerta `Jugador::mostrarInventario()` 📅 2026-09-25 🔽 #limpieza [completion:: 2026-09-14]
- [x] #13 Simplificar fórmula de XP en batallas a `exp_base * nivel_factor` 📅 2026-09-25 🔽 #balance [completion:: 2026-09-14]
- [x] #14 Reemplazar escalado cuadrático de estadísticas por curva controlada 📅 2026-09-25 🔽 #balance [completion:: 2026-09-14]
- [x] #15 Definir tabla de XP por nivel `EXP_TABLE[]` en `GameBalance.hpp` 📅 2026-09-25 🔽 #balance [completion:: 2026-09-14]
- [x] #18 Extraer `OverworldRenderer` utilizando `ScreenBuffer` para renderizar el mapa 📅 2026-09-25 🔽 #refactor #ui [completion:: 2026-09-14]

#### Sprint 4 — Dar personalidad (P4 - Nice to have)
- [x] #12 Implementar patrón Strategy `EnemyBehavior` para variedad de comportamiento 📅 2026-09-30 🔽 #sistema/enemigos [completion:: 2026-09-14]
- [x] #16 Agregar factor de nivel del jugador a la probabilidad de encuentros aleatorios 📅 2026-09-30 🔽 #balance [completion:: 2026-09-14]
- [x] #19 Mover `InventoryUI` como miembro re-usable de `BattleSystem` 📅 2026-09-30 🔽 #refactor [completion:: 2026-09-14]
- [x] #20 Mover `limpiarBuffer()` y `limpiarPantalla()` a `Platform::` 📅 2026-09-30 🔽 #limpieza [completion:: 2026-09-14]

### Resto de Sprints Previos
- [x] Transición entre niveles (nivel 1 → 2 → 3 → jefe final) 📅 2026-08-15 ⏫ #sistema/mapa  [completion:: 2026-09-08]
  - Modificar `handleTile('K')` para cargar siguiente nivel en vez de terminar el juego
  - Mantener estado del jugador entre niveles (nivel actual, stats, inventario)
  - CacheManager soporta múltiples mapas y partida unificada

- [x] Curva de dificultad verificable 📅 2026-08-15 ⏫  [completion:: 2026-09-08]
  - Nivel 1: enemigos con HP 25-50, ATK 7-12 → jugador nivel 1-2 puede ganar
  - Nivel 2: enemigos con HP 50-150, ATK 7-25 → jugador nivel 3-5 necesita buen equipo
  - Nivel 3: enemigos con HP 60-180, ATK 8-28 → jugador nivel 5-7 con armadura

- [x] Progresión por secciones y configuración `.meta` 📅 2026-09-08 ⏫ #sistema/mapa #sistema/enemigos
  - Límites de nivel por sección con interruptor de depuración `8`/`F8`.
  - Zonas asociadas a tiles con terreno, colores, símbolos y tablas de enemigos.
  - Terreno seguro sin encuentros y modificadores de estadísticas/XP.
  - Jefes asignados exclusivamente al `B` mediante `boss_id`.
  - Encuentros con cuatro pasos de gracia, multiplicador por mapa y crecimiento
    configurable hasta el 20%.
  - XP calculada por nivel/tier del enemigo y limitada al umbral actual.
  - Curación porcentual mediante `h`, `H` y `G`.

- [x] Reemplazar magic numbers por constexpr 📅 2026-08-01 🔺 #deuda-tecnica #sistema/combate  [completion:: 2026-07-23]
  Ver [[Sistemas/Combate#Sistema de nivelación]], [[Cambios#Magic Numbers]]
  Valores a reemplazar:
  - `Jugador.cpp:7`: 100, 15, 10, 1, 3 (stats base)
  - `Jugador.cpp:19`: 30 (curación poción)
  - `Jugador.cpp:132`: 50 (multiplicador salud/nivel)
  - `Jugador.cpp:134-135`: 5 (incremento ATK/DEF por nivel)
  - `Jugador.cpp:137`: 200 (umbral XP)
  - `Jugador.cpp:138`: 700 (XP nivel 3)
  - `batalla.cpp:206`: 50 (XP por batalla)

- [x] Configurar terreno por mapa en EncounterManager 📅 2026-08-01 🔺 #sistema/enemigos  [completion:: 2026-07-23]
  Ver [[Registro/Decisiones#Terreno no configurado]]
  Actualmente siempre queda en LLANURA (10%). Necesita:
  - Agregar campo de terreno al archivo de mapa o configurarlo por nivel
  - Llamar `encounterMgr.setTerreno()` al cargar cada nivel

- [x] Eliminar DataManager::guardarHeroe/cargarHeroe legacy 📅 2026-08-01 🔽 #deuda-tecnica #sistema/guardado  [completion:: 2026-07-23]
  Ver [[Sistemas/Guardado#DataManager]], [[Registro/Decisiones#DataManager legacy]]


## Cómo usar este archivo

1. **Al empezar una tarea**: Mover de "pendientes" a "activas"
2. **Al terminar**: Marcar `[x]`, mover a "completado", crear entrada en [[Cambios]]
3. **Al planner**: Revisar tareas activas, ajustar fechas si es necesario
