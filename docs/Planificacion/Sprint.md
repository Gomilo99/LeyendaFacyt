---
creado: 22/07/2026
modificado: 10/09/2026
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

## Sprint 1 — Cortar la hemorragia (P1 - Urgente)
- [x] #1 Extraer lógica de tiles y nivelación de `GameManager` hacia `TileHandler` y controladores específicos 📅 2026-09-15 ⏫ #arquitectura #deuda-tecnica [completion:: 2026-09-13]
- [x] #2 Reemplazar `handleTile()` hardcoded por registro `TileEvent` (`map<char, TileHandler>`) 📅 2026-09-15 ⏫ #arquitectura #sistema/mapa [completion:: 2026-09-13]
- [x] #3 Eliminar `suppressCout()`/`restoreCout()` refactorizando `atacar()` y `usarMagia()` para retornar `ActionResult` 📅 2026-09-15 ⏫ #refactor #sistema/combate [completion:: 2026-09-13]
- [x] #5 Eliminar `cin` de `Jugador::agregarObjeto()`, delegando la decisión de equipar a la UI 📅 2026-09-15 ⏫ #refactor #sistema/inventario [completion:: 2026-09-13]
- [x] #9 Agregar versión de formato de guardado (`j["version"] = 1`) y usar `.value()` en `CacheManager` 📅 2026-09-15 ⏫ #sistema/guardado [completion:: 2026-09-13]

## Sprint 2 — Ordenar la casa (P2 - Importante)
- [x] #4 Refactorizar `batalla()` para retornar `BattleResult` y mover post-combate a `BattleSystem` 📅 2026-09-20 🔺 #refactor #sistema/combate [completion:: 2026-09-13]
- [x] #6 Hacer que `Jugador::obtenerExperiencia()` retorne `LevelUpResult` sin `cout` directo 📅 2026-09-20 🔺 #refactor #sistema/combate [completion:: 2026-09-13]
- [x] #10 Guardar/cargar `saludMaxima` y `manaMaxima` explícitamente en `CacheManager` 📅 2026-09-20 🔺 #sistema/guardado [completion:: 2026-09-13]
- [x] #11 Centralizar guardado en `SaveController` / `CacheManager::guardarPartida` 📅 2026-09-20 🔺 #sistema/guardado [completion:: 2026-09-13]
- [x] #17 Mover configuración de terreno a `EncounterManager::configurarPorNivel` 📅 2026-09-20 🔺 #sistema/mapa [completion:: 2026-09-13]

## Sprint 3 — Pulir balance y UX (P3 - Mejora)
- [ ] #7 Eliminar función muerta `Jugador::mostrarInventario()` 📅 2026-09-25 🔽 #limpieza
- [ ] #13 Simplificar fórmula de XP en batallas a `exp_base * nivel_factor` 📅 2026-09-25 🔽 #balance
- [ ] #14 Reemplazar escalado cuadrático de estadísticas por curva controlada 📅 2026-09-25 🔽 #balance
- [ ] #15 Definir tabla de XP por nivel `EXP_TABLE[]` en `GameBalance.hpp` 📅 2026-09-25 🔽 #balance
- [ ] #18 Extraer `OverworldRenderer` utilizando `ScreenBuffer` para renderizar el mapa 📅 2026-09-25 🔽 #refactor #ui

## Sprint 4 — Dar personalidad (P4 - Nice to have)
- [ ] #12 Implementar patrón Strategy `EnemyBehavior` para variedad de comportamiento 📅 2026-09-30 🔽 #sistema/enemigos
- [ ] #16 Agregar factor de nivel del jugador a la probabilidad de encuentros aleatorios 📅 2026-09-30 🔽 #balance
- [ ] #19 Mover `InventoryUI` como miembro re-usable de `BattleSystem` 📅 2026-09-30 🔽 #refactor
- [ ] #20 Mover `limpiarBuffer()` y `limpiarPantalla()` a `Platform::` 📅 2026-09-30 🔽 #limpieza

## Completado este sprint


_(al terminar una tarea, moverla aquí)_

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
