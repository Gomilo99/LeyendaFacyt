---
creado: 22/07/2026
modificado: 08/09/2026
tipo: Avance
tags: # deuda-tecnica, idea-loca, bug-critico, bug, refactor
titulo: Desiciones
proyecto: "[[LeyendaFacyt]]"
area: Registro
estado: En progreso
prioridad: 0
dificultad: Media
version: 1.0.0
---
# Decisiones de Diseño

Registro de decisiones de diseño tomadas durante el desarrollo. Cada entrada documenta **qué** se decidió, **por qué**, y **qué alternativas** se consideraron.

Para logs de cambios técnicos, ver [[Cambios]].

---

## Decisiones

### Metadatos laterales `.meta` para mapas

**Fecha**: 2026-09-08
**Decisión**: Mantener la geometría en `nivelN.txt` y el balance/diseño en
`nivelN.meta`, usando zonas asociadas a tiles del mapa para casos como terrenos
y refugios.

**Motivo**: Evita mezclar reglas con la cuadrícula y permite cambiar el balance
sin rediseñar el mapa. El carácter real del terreno continúa definido en el
`.txt`; los estilos visuales del `.meta` se cargan como metadatos, pero el
renderizador actual todavía usa su tabla de símbolos y colores.

### Enemigos seleccionados por zona, no por nivel del jugador

**Fecha**: 2026-09-08
**Decisión**: La tabla ponderada de enemigos pertenece a la zona y el nivel del
héroe no cambia sus probabilidades. La zona puede modificar estadísticas y XP.

**Motivo**: Conserva la identidad de cada área y hace reproducible el balance
para cualquier jugador.

### Límite de nivel por sección

**Fecha**: 2026-09-08
**Decisión**: El límite bloquea la subida del héroe por sección. La XP se limita
al umbral actual y se muestra como `expMax/expMax`. `8`/`F8` desactiva el límite
durante depuración.

**Motivo**: La campaña controla el ritmo de progresión sin descartar XP ni
permitir que una zona temprana escale indefinidamente.

### [[Sistemas/Plataforma|Platform.hpp]] en vez de `#ifdef` dispersos

**Fecha**: 2026-06-18
**Decisión**: Crear un header-only `Platform.hpp` que abstraiga todas las diferencias entre Windows y Linux detrás de funciones inline simples.
**Alternativas consideradas**:
- `#ifdef _WIN32` en cada archivo → rechazado: duplicación, difícil de mantener
- Usar una librería como SDL → rechazado: demasiado pesado para un juego de terminal
- Solo soportar Windows → rechazado: limita portabilidad
**Resultado**: El resto del código nunca necesita `#ifdef`. El soporte multiplataforma se agrega en un solo archivo.
**Archivos afectados**: `lib/Platform.hpp` (nuevo), `src/main.cpp`, `src/Batalla.cpp`, `src/GameManager.cpp`, `src/Inventario.cpp`, `Makefile`
**Ver**: [[Cambios#Log 18/06/2026 — Soporte multiplataforma]]

---

### EnemyFactory data-driven en vez de switch hardcodeado

**Fecha**: 2026-06-15
**Decisión**: Cargar enemigos desde JSON y usar selección ponderada por ruleta en vez de un `switch` estático.
**Alternativas consideradas**:
- `switch` en `DataManager` → rechazado: cada nuevo enemigo requería recompilar
- Base de datos SQLite → rechazado: dependencia innecesaria para un juego CLI
**Resultado**: Agregar un enemigo es solo añadirlo a `json/enemigos.json`. No se toca código.
**Ver**: [[Sistemas/Enemigos#EnemyFactory]]

---

### ScreenBuffer con redibujado diferencial en vez de `limpiarPantalla()`

**Fecha**: 2026-06-15 (combate) / 2026-06-18 (inventario)
**Decisión**: Usar un buffer de doble capa que solo reescriba las líneas modificadas, en vez de limpiar y redraw completo.
**Alternativas consideradas**:
- `system("cls")` + redraw → rechazado: parpadeo visible, lento
- ncurses → rechazado: dependencia externa, no cross-platform nativo
**Resultado**: Transiciones sin parpadeo, renderizado eficiente. El mismo buffer se usa para combate e inventario.
**Ver**: [[Sistemas/Combate#ScreenBuffer]], [[Sistemas/Inventario]]

---

### CacheManager separado de DataManager

**Fecha**: 2026-06-15
**Decisión**: Separar la carga de datos (solo lectura, `DataManager`) de la persistencia de la partida (escritura, `CacheManager`).
**Alternativas consideradas**:
- Un solo manager para todo → rechazado: acoplamiento innecesario
- Escribir en `json/heroe.json` directamente → rechazado: contamina archivos originales
**Resultado**: `json/` y `mapas/` son read-only. Todo el estado mutable vive en `cache/`.
**Ver**: [[Sistemas/Guardado]]

---

### Inventario como overlay en vez de pantalla separada

**Fecha**: 2026-06-18
**Decisión**: El inventario se renderiza como overlay sobre el frame actual (combate o overworld) en vez de una pantalla completa nueva.
**Alternativas consideradas**:
- Pantalla completa con `limpiarPantalla()` → rechazado: pierde contexto, parpadeo
- Menú de texto con `cin` → rechazado: ya existía y era incómodo
**Resultado**: El inventario opera sobre el mismo espacio de la terminal, el frame se restaura al salir.
**Ver**: [[Sistemas/Inventario#Integración desde combate]]

---

### Magic Numbers — Pendiente de resolución

**Fecha**: 2026-06-10 (documentado), pendiente
**Decisión**: Los valores numéricos en el código (curación=30, XP=nivel*50, etc.) están hardcodeados como magic numbers.
**Estado**: Documentados en [[Cambios#Magic Numbers]] pero no reemplazados.
**Plan**: Reemplazar por `constexpr` con nombre (`SALUD_POR_NIVEL`, `POCION_CURACION`, etc.) antes de agregar más features de balanceo.
**Ver**: [[Planificacion/Roadmap#Objetivo 50%]]

---

### Terreno nunca configurado — Bug conocido

**Fecha**: 2026-06-15 (detectado)
**Decisión**: `EncounterManager` tiene 4 terrenos (CAMINO, LLANURA, BOSQUE, MAZMORRA) pero `GameManager` nunca llama `setTerreno()`. Siempre queda en LLANURA por defecto.
**Impacto**: Todos los mapas tienen la misma probabilidad de encuentro (10%), sin importar la zona.
**Plan**: Configurar terreno por mapa o por tile en `GameManager`.
**Ver**: [[Sistemas/Enemigos#EncounterManager]]

---

### DataManager::guardarHeroe/cargarHeroe legacy

**Fecha**: 2026-06-15
**Decisión**: `DataManager` aún tiene `cargarHeroe()` y `guardarHeroe()` que escriben en `json/heroe.json`, pero fueron reemplazados por `CacheManager`.
**Estado**: Código muerto. `batalla()` ya usa `CacheManager`.
**Plan**: Eliminar las funciones legacy de `DataManager`.
**Ver**: [[Sistemas/Guardado#DataManager]]
