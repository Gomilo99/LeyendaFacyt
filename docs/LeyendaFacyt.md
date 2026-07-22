---
alias:
tags:
  - gamedev
  - JRPG
creado: 20/06/2026
modificado: 22/07/2026
estado: En progreso
tipo: Proyecto
base:
  - "[[02-Proyectos]]"
  - "[[A-GameDev]]"
fecha_inicio: 2026-06-19
fecha_fin_prevista: Finales 2026
progreso: 50%
---
# [[LeyendaFacyt_backup]]

## Descripción
Es un videojuego basado en terminal hecho en c++ que nació como prueba personal para entender la programación orientada a objetos y mejorar mis habilidades de programación. Es un juego de rol donde controlas a un personaje que debe enfrentar a diversos enemigos en un mapa con vista de pájaro (top-down) mientras subes de nivel y obtienes objetos.

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

## Objetivos
- [x] Objetivo 25% - Creación y funcionamiento general del videojuego  [completion:: 2026-06-19]
- [x] Objetivo 40% - Renovación y creación de sistemas responsive y sistemas más funcionales.  [completion:: 2026-06-19]
- [ ] Objetivo 50% - Establecimiento de diseño de niveles, rutas de progreso, progresión y balanceo de niveles y construcción del mundo.
	- [ ] Transición entre niveles funcional (nivel 1 -> nivel 2 al derrotar jefe)
	- [ ] 3+ mapas con diseños distintos y temáticas
	- [ ] Encuentros aleatorios ajustados por terreno (el código ya tiene `EncounterManager` con 4 terrenos, pero nunca se llama `setTerreno()` desde `GameManager`).
	- [ ] Curva de dificultad ascendente escalable (jugar nivel 1->2->3 sin morir es posible pero desafiante).
	- [ ] Magic numbers reemplazados por `constexpr`
- [ ] Objetivo 75% - Integración de animaciones (CLI) como movimientos de pantalla, efectos de sonido, música y el establecimiento de una historia. Establecimiento de Subjefes y jefe final.
- [ ] Objetivo 85% - Posible mejora en historia, creación de npc con tienda interactiva, mensajes predeterminados y puzzles.
- [ ] Objetivo 95% - Revisión total de gameplay, historia, responsive de la ui y sistemas y sobre todo la mejora de las rutas de juego: más diversión. Corrección de errores y playtesting.
- [ ] Objetivo 100% - Creación de arte para portada y publicación.

**Fecha prevista de entrega**: Finales de 2026
## Tareas
- [ ] Continuidad entre niveles - nivel2.txt existe pero **el juego siempre carga nivel 1**. No hay transición.
- [ ] Acción Defender - Declarada en el roadmap pero nunca implementada
- [ ] Armadura equipable - Solo hay slot de arma
- [ ] Terreno configurable - EncounterManager tiene 4 terrenos pero GameManager nunca llama setTerreno()

## Notas y Recursos
- [[Nota relacionada 1]]
- [[Recurso útil 1]]

## Próximos Pasos
- Principalmente, empieza por establecer rutas de progreso divertidas, alineadas a un balanceo total con posible recorte de los enemigos y objetos. 
- A esto se le puse sumar la creación y conexión de nuevos niveles, delimitación de zonas para la generación de enemigos y posible escalado adicional de niveles y dificultad de enemigos. 
- Se podría integrar escritura letra a letra para el encuentro con jefes, mejorar el log para no que no se pierda la cantidad de daño hecho al enemigo.
- También se podría integrar nuevos mini jefes con algunos objetos nuevos
- Tal ves un sistema mejorado para los objetos con rarezas: objetos más raros tienen colores diferentes
- Posible reconstrucción de las bases de datos de los objetos y enemigos para facilitar el balanceo y la construcción de nuevos registros.