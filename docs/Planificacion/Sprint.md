---
creado: 22/07/2026
modificado: 22/07/2026
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

> **Sprint**: Julio 2026 — Fase de multi-nivel y limpieza de código
> **Objetivo**: Completar la transición entre niveles y eliminar deuda técnica básica

---

## Tareas activas

- [ ] Reemplazar magic numbers por constexpr 📅 2026-08-01 🔺 #deuda-tecnica #sistema/combate
  Ver [[Sistemas/Combate#Sistema de nivelación]], [[Cambios#Magic Numbers]]
  Valores a reemplazar:
  - `Jugador.cpp:7`: 100, 15, 10, 1, 3 (stats base)
  - `Jugador.cpp:19`: 30 (curación poción)
  - `Jugador.cpp:132`: 50 (multiplicador salud/nivel)
  - `Jugador.cpp:134-135`: 5 (incremento ATK/DEF por nivel)
  - `Jugador.cpp:137`: 200 (umbral XP)
  - `Jugador.cpp:138`: 700 (XP nivel 3)
  - `batalla.cpp:206`: 50 (XP por batalla)

- [ ] Configurar terreno por mapa en EncounterManager 📅 2026-08-01 🔺 #sistema/enemigos
  Ver [[Registro/Decisiones#Terreno no configurado]]
  Actualmente siempre queda en LLANURA (10%). Necesita:
  - Agregar campo de terreno al archivo de mapa o configurarlo por nivel
  - Llamar `encounterMgr.setTerreno()` al cargar cada nivel

- [ ] Eliminar DataManager::guardarHeroe/cargarHeroe legacy 📅 2026-08-01 🔽 #deuda-tecnica #sistema/guardado
  Ver [[Sistemas/Guardado#DataManager]], [[Registro/Decisiones#DataManager legacy]]

---

## Tareas pendientes (próximo sprint)

- [ ] Transición entre niveles 📅 2026-08-15 ⏫ #sistema/mapa
  Requiere:
  - Modificar `handleTile('K')` para cargar siguiente nivel en vez de terminar el juego
  - Mantener estado del jugador entre niveles (nivel actual, stats, inventario)
  - CacheManager debe soportar múltiples mapas
  - Crear `mapas/nivel3.txt` si no existe

- [ ] Curva de dificultad verificable 📅 2026-08-15 ⏫
  Verificar que:
  - Nivel 1: enemigos con HP 25-50, ATK 7-12 → jugador nivel 1-2 puede ganar
  - Nivel 2: enemigos con HP 50-150, ATK 7-25 → jugador nivel 3-5 necesita buen equipo
  - Nivel 3: enemigos con HP 60-180, ATK 8-28 → jugador nivel 5-7 con armadura

---

## Completado este sprint

_(vacío — al terminar una tarea, moverla aquí)_

---

## Cómo usar este archivo

1. **Al empezar una tarea**: Mover de "pendientes" a "activas"
2. **Al terminar**: Marcar `[x]`, mover a "completado", crear entrada en [[Cambios]]
3. **Al planner**: Revisar tareas activas, ajustar fechas si es necesario
