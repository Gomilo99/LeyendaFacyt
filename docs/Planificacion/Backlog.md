# Backlog #plan/backlog

Ideas y mejoras futuras. No están planificadas para un sprint específico. Cuando se planifiquen, mover a [[Sprint]].

---

## Gameplay

- [ ] Acción Defender en combate (reduce daño recibido un turno)
- [ ] Armadura como equipable (slot separado de arma)
- [ ] Subjefes por nivel con mecánicas únicas
- [ ] Sistema de crítico (daño multiplicado por probabilidad)
- [ ] Objetos con rarezas (común, raro, épico, legendario) con colores diferentes
- [ ] NPC con tienda interactiva (compra/venta de objetos)
- [ ] Sistema de moneda/dinero
- [ ] Puzzles básicos (puertas que requieren llaves, presiones, etc.)
- [ ] Habilidades passivas del jugador (se desbloquean por nivel)
- [ ] Enemigos con habilidades especiales (no solo ataque básico)

## UI/UX

- [ ] Escritura letra a letra para textos importantes (jefes, historia)
- [ ] Log de combate mejorado (historial scrollable, no se pierde daño)
- [ ] Colores por rareza de objetos en inventario
- [ ] Pantalla de estadísticas finales al completar el juego
- [ ] Transiciones animadas entre estados (fade in/out con ANSI)
- [ ] Mini-mapa en la esquina de la pantalla
- [ ] Indicador de dirección del jugador en el mapa

## Audio

- [ ] Investigar opciones de audio cross-platform (SDL_mixer, beep, etc.)
- [ ] Efectos de sonido básicos (ataque, defender, curar, victoria, derrota)
- [ ] Música de fondo por zona (si viable)

## Contenido

- [ ] Narrativa base del juego (¿quién es el héroe? ¿por qué pelea?)
- [ ] Textos entre niveles / transiciones narrativas
- [ ] 5+ mapas con temáticas distintas
- [ ] Arte de portada ASCII
- [ ] README completo con screenshots y guía de compilación

## Técnico

- [ ] Extraer UI de Jugador (quitar cout/cin de las clases de modelo)
- [ ] Tests unitarios básicos (al menos para fórmulas de combate y nivelación)
- [ ] CI/CD básico (compilar en Windows + Linux en cada push)
- [ ] Documentación Doxygen generada automáticamente

## Archivos de referencia

- Ver [[Gemini]] para el prompt original de diseño
- Ver [[Cambios]] para el historial de cambios técnicos
