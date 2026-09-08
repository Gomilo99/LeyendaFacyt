---
creado: 22/07/2026
modificado: 08/09/2026
tipo: Avance
tags: # deuda-tecnica, idea-loca, bug-critico, bug, refactor
titulo: Roadmap
proyecto: "[[LeyendaFacyt]]"
area: Planificacion
estado: En progreso
prioridad: 0
dificultad: Media
version: 1.0.0
---
# Roadmap

Visión general del progreso del proyecto. Cada objetivo tiene tareas concretas con Tags de Tasks plugin.

Para el trabajo actual del sprint, ver [[Sprint]]. Para ideas futuras, ver [[Backlog]].

---

## Estado actual

| Objetivo | Estado | Completado |
|----------|--------|------------|
| 25% — Funcionamiento general | ✅ | 2026-06-19 |
| 40% — Sistemas responsive | ✅ | 2026-06-19 |
| 50% — Multi-nivel y balanceo | 🔄 En progreso | — |
| 75% — Historia, animaciones, subjefes | ⏳ Pendiente | — |
| 85% — NPC tienda, puzzles | ⏳ Pendiente | — |
| 95% — Polish y playtesting | ⏳ Pendiente | — |
| 100% — Arte y publicación | ⏳ Pendiente | — |

**Fecha prevista de entrega**: Finales de 2026

---

## Objetivo 25% — Funcionamiento general ✅

> Creación y funcionamiento general del videojuego.

- [x] Sistema de combate por turnos funcional
- [x] JSON-driven (objetos, enemigos)
- [x] Inventario con equipamiento de armas
- [x] Loot probabilístico por enemigo
- [x] Sistema de nivelación con stats escalables
- [x] Mapa cargado desde archivo
- [x] Movimiento WASD con colisiones
- [x] Encuentros aleatorios al moverte
- [x] Sistema de magia/MP
- [x] Condición de victoria por llave
- [x] Máquina de estados (FSM)

**Completado**: 2026-06-19
**Ver**: [[Sistemas/Combate]], [[Sistemas/Enemigos]], [[Sistemas/Mapa]]

---

## Objetivo 40% — Sistemas responsive ✅

> Renovación y creación de sistemas responsive y sistemas más funcionales.

- [x] Soporte multiplataforma (Windows + Linux) — [[Sistemas/Plataforma]]
- [x] Separación de clases, hpp sobre cpp
- [x] UI renovada con movement responsive
- [x] Pantalla principal y menús
- [x] Inventario renovado con scrolling y visualización de datos
- [x] Colores para menús y mapas (paredes, enemigos, jefes, curación)
- [x] Enemigos renovados: encuentros aleatorios + EnemyFactory
- [x] Menú de combate renovado con ScreenBuffer
- [x] Sistema de guardado y carga de partida — [[Sistemas/Guardado]]
- [x] Ataques de maná (magia funcional)
- [x] Visualización de arte del enemigo, barras de vida, log de combate

**Completado**: 2026-06-19
**Ver**: [[Sistemas/Guardado]], [[Sistemas/Inventario]]

---

## Objetivo 50% — Multi-nivel y balanceo 🔄

> Establecimiento de diseño de niveles, rutas de progreso, progresión y balanceo de niveles y construcción del mundo.

- [x] Transición entre niveles (nivel 1 → 2 → 3 → jefe final) 📅 2026-08-15 ⏫ #plan/sprint #sistema/mapa [completion:: 2026-09-08]
  Ver [[Sistemas/Mapa#Mapas actuales]] y [[Sistemas/Guardado]].
- [x] Configurar terreno por mapa en EncounterManager 📅 2026-08-01 🔺 #plan/sprint #sistema/enemigos  [completion:: 2026-09-07]
  Ver [[Registro/Decisiones#Terreno no configurado]].
- [x] Reemplazar magic numbers por constexpr 📅 2026-08-01 🔺 #plan/sprint #deuda-tecnica  [completion:: 2026-09-07]
  Ver [[Sistemas/Combate#Sistema de nivelación]], [[Cambios#Magic Numbers]].
- [x] Curva de dificultad verificable entre niveles 📅 2026-08-15 ⏫ #plan/sprint [completion:: 2026-09-08]
- [x] 3+ mapas con diseños distintos y temáticas 📅 2026-08-30 🔺 #plan/sprint #sistema/mapa [completion:: 2026-09-08]
- [x] Eliminar DataManager::guardarHeroe/cargarHeroe legacy 📅 2026-08-01 🔽 #deuda-tecnica  [completion:: 2026-09-07]
  Ver [[Registro/Decisiones#DataManager legacy]].

---

## Objetivo 75% — Historia, animaciones, subjefes ⏳

> Integración de animaciones (CLI) como movimientos de pantalla, efectos de sonido, música y el establecimiento de una historia. Establecimiento de Subjefes y jefe final.

### Gameplay (prioritario)
- [ ] Acción Defender en combate 📅 2026-09-15 🔺 #plan/backlog #sistema/combate
- [ ] Armadura como equipable 📅 2026-09-30 🔺 #plan/backlog #sistema/inventario
- [ ] Subjefes por nivel con mecánicas únicas 📅 2026-10-15 ⏫ #plan/backlog
- [ ] Jefe final con lore y transiciones 📅 2026-10-30 ⏫ #plan/backlog

### UI/UX
- [ ] Escritura letra a letra para encuentros con jefes 📅 2026-09-30 🔺 #plan/backlog
- [ ] Log de combate mejorado (no perder daño) 📅 2026-09-15 🔺 #plan/backlog #sistema/combate
- [ ] Colores por rareza de objetos 📅 2026-10-15 🔺 #plan/backlog #sistema/inventario

### Audio
- [ ] Investigar opciones de audio cross-platform 📅 2026-09-01 🔺 #plan/backlog
- [ ] Efectos de sonido básicos (si viable) 📅 2026-10-15 🔽 #plan/backlog

### Historia
- [ ] Establecer narrativa base del juego 📅 2026-09-15 ⏫ #plan/backlog
- [ ] Textos entre niveles / transiciones narrativas 📅 2026-10-15 ⏫ #plan/backlog

---

## Objetivo 85% — NPC tienda, puzzles ⏳

> Posible mejora en historia, creación de npc con tienda interactiva, mensajes predeterminados y puzzles.

- [ ] NPC con tienda interactiva 📅 2026-11-01 ⏫ #plan/backlog
- [ ] Sistema de moneda/dinero 📅 2026-11-01 ⏫ #plan/backlog
- [ ] Mensajes predeterminados para NPCs 📅 2026-11-15 🔺 #plan/backlog
- [ ] Puzzles básicos 📅 2026-11-30 🔺 #plan/backlog
- [ ] Mejora en historia y diálogos 📅 2026-11-15 🔺 #plan/backlog

---

## Objetivo 95% — Polish y playtesting ⏳

> Revisión total de gameplay, historia, responsive de la ui y sistemas y sobre todo la mejora de las rutas de juego: más diversión. Corrección de errores y playtesting.

- [ ] Playtesting completo (1-3 jugadores) 📅 2026-12-01 ⏫ #plan/backlog
- [ ] Balance final de dificultad 📅 2026-12-01 ⏫ #plan/backlog
- [ ] Corrección de errores encontrados 📅 2026-12-15 ⏫ #plan/backlog
- [ ] Revisión de UI responsive 📅 2026-12-15 🔺 #plan/backlog
- [ ] Revisión de historia coherente 📅 2026-12-15 🔺 #plan/backlog

---

## Objetivo 100% — Arte y publicación ⏳

> Creación de arte para portada y publicación.

- [ ] Arte de portada ASCII 📅 2026-12-20 ⏫ #plan/backlog
- [ ] README completo con screenshots 📅 2026-12-20 🔺 #plan/backlog
- [ ] Distribución final (make dist) 📅 2026-12-30 ⏫ #plan/backlog
- [ ] Publicación (GitHub release / itch.io) 📅 2026-12-30 ⏫ #plan/backlog

---

## Riesgos conocidos

1. **Tiempo**: Universidad + Sistemas Operativos limitan horas de desarrollo
2. **Abandono**: Plazos laxos y largas esperas entre sesiones
3. **Scope creep**: El audio y la primera persona son features de alto costo/beneficio

Ver [[LeyendaFacyt#Justificación]] para el contexto completo.
