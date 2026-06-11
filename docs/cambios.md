1. Se arreglaron problemas y bugs
    - Se cambio de recursividad a un while para la lógica de las batallas
    - Ahora si se carga el json con los datos del heroe.
    - Se modificó la entrada de datos a la función de batalla de "por valor" a "por referencia" para reducir el uso de memoria y la carga en cada iteración.
    - Se cambio los NULL -> nullptr
    - Se modificó el uso de `system("clear")` para usar caracteres ANSI y que sea más portable.
2. Se agrego la funcion de guardarHeroe() para mantener los datos del personaje
3. Se modificaron y se eliminó `using namespace std;`, se añadieron todas las llamadas a la librería std.
4. Modificación estructura del proyecto.

### Reestructuración
Listo. Estructura final del proyecto:

    Raiz/
    ├── Objeto.hpp         # Clases Objeto, Arma, Pocion, ObjClave, Drop
    ├── Personaje.hpp      # Clase base Personaje
    ├── Jugador.hpp        # Declaracion de Jugador
    ├── Jugador.cpp        # Implementacion de Jugador
    ├── Enemigo.hpp        # Declaracion de Enemigo
    ├── Enemigo.cpp        # Implementacion de Enemigo
    ├── Batalla.hpp        # Declaracion de funciones (JSON, batalla, etc.)
    ├── Batalla.cpp        # Implementacion de batalla y JSON
    ├── main.cpp           # Solo main()
    ├── batalla.cpp        # (reemplazado por nota)
    ├── lib/
    │   ├── json.hpp       # Libreria nlohmann/json
    │   ├── objeto.hpp     # → redirige a Objeto.hpp
    │   └── Personaje.hpp  # → redirige a Personaje.hpp
    ├── objetos.json
    ├── enemigos.json
    └── heroe.json


### Mejoras
Estado actual del proyecto:

Lo que tienes ahora es un módulo de combate sólido y funcional, pero no es un dungeon crawler completo. Es solo el subsistema de batalla del proyecto soñado. Los puntos fuertes actuales:

✅ Sistema de combate por turnos funcional
✅ JSON-driven (objetos, enemigos variables)
✅ Inventario con equipamiento de armas
✅ Loot probabilístico por enemigo
✅ Sistema de nivelación con stats escalables
✅ Separación en archivos limpia
✅ Sin dependencias Windows
Lo que falta para llegar al dungeon crawler:

Característica	Estado
Mapa cargado desde archivo	❌ No existe
Movimiento WASD con colisiones	❌ No existe
Vista primera persona ASCII	❌ No existe
Encuentros aleatorios al moverte	❌ No existe
Acción "Defender" en combate	❌ Solo atacar/usar poción
Sistema de magia/MP	❌ No existe
Armadura como equipable	❌ Solo arma
Condición de victoria (llave)	❌ Solo jefe final
Máquina de estados (FSM)	❌ Un solo flujo lineal
Crítica al diseño de gemini.md:

La vista en primera persona es el verdadero desafío — El prompt dice "Aquí programarías la lógica" como si fuera trivial. Simular un pasillo 3D en ASCII requiere dibujar paredes en perspectiva según la dirección que mires y la distancia a las paredes. Es lo más complejo del proyecto, no un detalle menor.
El diseño usa new sin delete limpio — currentEnemy = new Enemy(...) y luego delete currentEnemy. En C++ moderno debería ser std::unique_ptr o std::shared_ptr como ya tienes.
No es data-driven — Propone enemigos hardcodeados en vez de cargarlos desde JSON como ya haces. Tu enfoque actual es superior.
using namespace std; y system("cls") — Ya corregiste esto. El diseño de gemini.md está atrás en ese aspecto.
Fórmulas de combate — Las de gemini.md son genéricas; las tuyas ya están implementadas y funcionando.
Mi recomendación de evolución:

Tu código actual ya es el módulo COMBAT del dungeon crawler. La evolución natural sería:

Fase 1: Agregar Mapa (2D top-down)
  → MapSystem que carga .txt, matriz de tiles
  → Jugador se mueve con WASD, colisiones con '#'

Fase 2: Transición mapa → combate
  → Al pisar 'E' en el mapa, cargar enemigo desde JSON
  → Llamar a tu batalla() existente
  → Al ganar/perder, volver al mapa

Fase 3: Vista primera persona
  → Renderizar 3 líneas de visión (adelante, izq, der)
  → Mapear tiles del mapa a arte ASCII

Fase 4: Pulir
  → Agregar acción Defender, magia/MP
  → Equipar armadura
  → Condición de victoria con llave
Tu base actual es buena porque el combate ya funciona con datos reales. No tires nada — la batalla que tienes es mejor que lo que propone gemini.md. Solo falta envolverla en un sistema de mapa y estados.

### Routmap
Aquí está el diseño conceptual fusionado y el roadmap para construirlo.

Arquitectura Fusionada
            ┌─────────────────────────────────────────┐
            │              GameEngine                  │
            │  (Máquina de Estados: FSM)               │
            │                                          │
            │  EXPLORE ──encuentro──▶ COMBAT           │
            │     ▲                    │               │
            │     │◀─victoria/derrota──┘               │
            │     │                    │               │
            │     └──INVENTARIO──┘     │               │
            │                    GAME_OVER / WIN       │
            └──────┬──────┬──────┬──────┬──────────────┘
                   │      │      │      │
            ┌──────┘┌─────┘┌─────┘┌─────┘
            ▼      ▼      ▼      ▼
       MapSystem │  Combat  │  Input   │  Render
       (tu nuevo)│  (tu código actual) │  (ASCII)

Estructura de Archivos Propuesta
Raiz/
├── main.cpp
├── GameEngine.hpp/.cpp      ← NUEVO (FSM)
│
├── Objeto.hpp               ← EXISTE (mejorar)
├── Personaje.hpp             ← EXISTE
├── Jugador.hpp/.cpp          ← EXISTE (mejorar)
├── Enemigo.hpp/.cpp          ← EXISTE
│
├── Armadura.hpp              ← NUEVO (slot equipable)
│
├── Inventario.hpp/.cpp       ← NUEVO (extraído de Jugador)
├── Magia.hpp                 ← NUEVO (hechizos)
│
├── Mapa.hpp/.cpp             ← NUEVO (carga + colisiones)
├── Vista.hpp/.cpp            ← NUEVO (render primera persona)
│
├── Batalla.hpp/.cpp          ← EXISTE (refactorizar)
│
├── lib/json.hpp
├── objetos.json              ← EXISTE (extender)
├── enemigos.json             ← EXISTE
├── mapas/
│   ├── nivel1.txt            ← NUEVO
│   └── nivel2.txt
└── heroe.json

Leyenda del Mapa (nivel1.txt)
################
#......E.......#
#..............#
#..E......K....#
#..............#
#......P.......#
################

Símbolo	Significado
#	Pared
.	Suelo transitable
P	Posición inicial del jugador
E	Spawn de enemigo (aleatorio del JSON por nivel)
K	Llave mágica (victoria)
H	Poción curativa en el suelo
T	Trampa (daño al pisar)

Mapa de Transiciones (Flujo del Juego)
```
  EXPLORE
    │
    ├─ WASD ──────────────→ se mueve por el mapa
    ├─ pisa 'E' ──────────→ transición a COMBAT
    ├─ pisa 'K' ──────────→ WIN (game over victorioso)
    ├─ pisa 'H' ──────────→ curar y eliminar del mapa
    ├─ pisa 'T' ──────────→ recibir daño
    └─ tecla 'I' ─────────→ INVENTARIO (pausa)

  COMBAT (tu código actual, modificado)
    │
    ├─ gana batalla ──────→ loot + exp → EXPLORE
    ├─ pierde (vida 0) ───→ GAME_OVER
    └─ huye (-80% vida) ──→ vuelve a EXPLORE

  INVENTARIO
    │
    ├─ equipar/beber ─────→ vuelve a EXPLORE
    └─ salir ─────────────→ vuelve a EXPLORE

  WIN  ───→ "Has escapado de la mazmorra"
  GAME_OVER ───→ "Has muerto"
```

Roadmap por Fases
Fase 0 — Base actual (ya lista)
Combate funcional, JSON, inventario, equipamiento, nivelación
Se compila y juega
Fase 1 — Sistema de Mapa 2D
Archivos nuevos:

Mapa.hpp / Mapa.cpp
mapas/nivel1.txt
Qué hace:

Carga un .txt a una matriz std::vector<std::string>
El jugador tiene posición (x, y) y se mueve con WASD
Colisiones con # (no puede atravesar)
Renderizado top-down sencillo para debug:
```
################
#......E.......#
#..@...........#
#..............#
################
```
Cambios en tu código:

Jugador gana getX(), getY(), setPos(int x, int y)
Personaje gana getPosX(), getPosY()
Prueba: Mover un @ por el mapa sin salirte.

Fase 2 — Integrar Combate con Mapa
Qué cambia:

GameEngine con máquina de estados (enum GameState)
Al pisar E, se genera un enemigo desde tu JSON actual y se llama a batalla()
Al terminar el combate, vuelve al mapa en la misma posición
El enemigo se marca como derrotado (no reaparece en esa casilla)
Cambios en tu código:

batalla() ya recibe Jugador& — funciona igual
Solo agregas la transición de estado en GameEngine
Prueba: Caminar por el mapa, encontrar enemigo, combatir, volver al mapa.

Fase 3 — Vista Primera Persona ASCII
Archivos nuevos:

Vista.hpp / Vista.cpp
Qué hace:

En lugar del mapa top-down, dibuja 3 columnas ASCII:
```
               █████████████
               █  PASILLO  █
               █  OSCURO   █
               █     ██    █
               █     ██    █
      █████    █     ██    █    █████
      █ PARED█ █     ██    █ █ PARED█
      ███████ ███████████████ ███████
```
La lógica: según el tile adelante, izquierda y derecha en el mapa, dibuja pared o espacio abierto
Usar caracteres █ ░ ▓ █ para texturas
Algoritmo básico:
```cpp
void renderizarVista(const Mapa& mapa, int x, int y, char direccion) {
    char adelante = mapa.getTile(x, y, direccion, 1);
    char izquierda = mapa.getTile(x, y, direccion, 2);
    char derecha = mapa.getTile(x, y, direccion, 3);
    // Dibujar ASCII según lo que haya a 1, 2, 3 casillas
}
```
Prueba: Reemplazar el render top-down por la vista primera persona.

Fase 4 — Acción Defender y Magia
Archivos nuevos:

Magia.hpp — estructura de hechizos (nombre, costeMP, daño, tipo)
Objetos mágicos en objetos.json
Cambios en Jugador:

Añadir mp, mpMaximo
Añadir defensaTemporal (para acción Defender)
Nuevos métodos: defender(), usarMagia(Hechizo, Enemigo&)
Cambios en batalla():

Menú extendido:
```
1. Atacar
2. Defender (reduce daño recibido este turno)
3. Magia (gasta MP)
4. Usar poción
5. Huir (probabilidad, penalización)
```
Cambios en Personaje:

Añadir getMP(), setMP()
Turno del enemigo: ataque simple o usar habilidad según su JSON
Fase 5 — Armadura y Equipamiento Completo
Archivos nuevos:

Armadura.hpp — clase derivada de Objeto, con defensa
Cambios en objetos.json:
```json
"armadura": [
    {
        "nombre": "Armadura de Cuero",
        "descripcion": "Cuero endurecido, huele a goblin.",
        "defensa": 5
    }
]
```
Cambios en Jugador:

Slot armaduraEquipada (similar a armaEquipada)
defensa base + armadura al calcular daño recibido
Al dropear armadura, preguntar si quiere equiparla
Cambios en cargarObjetosDesdeJSON:

Leer sección "armadura" del JSON
Fase 6 — Condición de Victoria y Múltiples Niveles
Al pisar K, transición a WIN
Mostrar pantalla de victoria con estadísticas finales
Opcional: al ganar, cargar nivel2.txt más difícil
Persistencia: guardarHeroe() al salir de cada nivel
Obstáculos: puertas cerradas que requieren llave del piso anterior
Fase 7 — Pulido Final
Barras de vida horizontales con ASCII: [██████░░░░]
Efectos de sonido con \a (beep)
Mensajes con color ANSI (\033[31m para rojo, \033[32m verde)
Animación de texto lento (sleep_for + cout)
Transiciones suaves entre estados
Diagrama de Clases Fusionado
```
Objeto
  ├── Arma (dano)
  ├── Armadura (defensa)      ← NUEVO
  ├── Pocion (curacion)
  └── ObjClave

Personaje
  ├── atacar() = 0
  ├── recibirDano()
  ├── getMP() / setMP()       ← NUEVO
  └── getPosX() / getPosY()   ← NUEVO
      │
      ├── Jugador
      │   ├── inventario      ← extraer a clase Inventario
      │   ├── armaEquipada
      │   ├── armaduraEquipada ← NUEVO
      │   ├── mp / mpMaximo   ← NUEVO
      │   ├── defender()
      │   └── usarMagia()
      │
      └── Enemigo
          ├── loot1, loot2
          └── getSprite()     ← NUEVO (ASCII art)
```
Drop (struct) → Objeto + probabilidad
```
Mapa                 ← NUEVO
  ├── cargar("archivo.txt")
  ├── getTile(x, y)
  ├── esTransitable(x, y)
  └── getAlto() / getAncho()

Vista                ← NUEVO
  └── renderizarPrimeraPersona(mapa, x, y, direccion)

GameEngine           ← NUEVO
  ├── estado: GameState
  ├── init()
  ├── updateAndRender()
  └── FSM: EXPLORE → COMBAT → WIN/GAME_OVER

Batalla              ← EXISTE (refactorizar)
  └── batalla(Jugador&, mapaEnemigos)
```
Resumen Estratégico
Lo que ya tienes y sirve:

Todo el sistema de objetos, combate, loot, inventario, nivelación
JSON como fuente de datos
Separación en archivos
Lo que tomas de gemini.md:

FSM del GameEngine
Mapa cargado desde archivo
Vista primera persona (aunque hay que implementarla bien)
Acciones Defender, Magia, Huir
Armadura como equipable
Condición de victoria por llave
Orden recomendado de implementación:
```
Mapa 2D ──▶ Integrar combate ──▶ Vista 1ra persona ──▶ Defender/Magia
    └──▶ Armadura ──▶ Victoria/multi-nivel ──▶ Pulido
```
Cada fase produce un juego jugable desde el primer momento. No necesitas terminar todo para probar.