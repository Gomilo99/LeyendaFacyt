# Sistema de Inventario #sistema/inventario

> Archivos: `lib/Inventario.hpp`, `src/Inventario.cpp`

El inventario usa la misma arquitectura que el [[Combate|combate]]: `ScreenBuffer` (56x22), renderizado por secciones, input por teclado con navegación W/S/A/D/SPACE/Q. Opera como overlay independiente sobre el frame actual.

## Arquitectura

```
InvRenderer ← InventoryUI
    ↑              ↑
ScreenBuffer   Jugador (stats e inventario)
```

Ver también: [[Combate]] (integra el inventario como acción), [[Guardado]] (el inventario se persiste en caché).

## Clases

### InvRenderer

Dibuja cada sección del inventario sobre un `ScreenBuffer`:

| Función | Líneas | Contenido |
|---------|--------|-----------|
| `drawBackground()` | 0-1 | Título "=== LEYENDA DEL CAMPUS - INVENTARIO ===" (BYELLOW), separador (CYAN) |
| `drawCategoryTabs()` | 2 | Tres pestañas: `[Armas] [Consumibles] [Clave]`. Activa en BYELLOW |
| `drawItemList()` | 4-11 | Box 26x9. Lista scrolleable con `> nombre xN` para seleccionado. Scroll indicators `▲`/`▼` |
| `drawItemDetails()` | 4-11 | Box 25x9. Panel derecho: tipo, stats, descripción con word-wrap |
| `drawPlayerStats()` | 14-16 | Box 52x3. HP bar + MP bar + ATK/DEF/NIV + EXP + arma equipada |
| `drawFooter()` | 21 | Controles centrados en CYAN |
| `renderAll()` | — | Compone todo y llama a `buf.render()` |

### InventoryUI

Orquestador con máquina de estados:

```cpp
enum class InvState {
    BROWSING,        // navegando la lista — W/S items, A/D categorías, SPACE acción, Q salir
    ITEM_ACTIONS,    // submenú — SPACE ejecuta, Q o ESC vuelve
    CONFIRM_ACTION,  // reservado para futura confirmación
    CLOSED           // salir del bucle
};
```

## Layout del frame

```
L0:  === LEYENDA DEL CAMPUS - INVENTARIO ===   (BYELLOW)
L1:  ────────────────────────────────────────   (CYAN)
L2:   [Armas] [Consumibles] [Clave]            (pestañas)
L3:  ┌──────────────────┬────────────────────┐
L4:  │ > Espada Leyenda  │  Daño: +20         │
L5:  │   Daga Plateada   │  "Poderosa espada" │
L6:  │   Bastón Arcano   │                     │
L7:  │   Hacha de Guerra │  Equipada: Daga    │
L8:  │   Espada Gallo    │  (+8)              │
L9:  │                ▲ │                     │
L10: │                ▼ │                     │
L11: └──────────────────┴────────────────────┘
L12: ┌─ HEROE ──────────────────────────────────┐
L13: │ HP ▓▓▓▓▓▓▓▓▓░░ 30/40  MP ▓▓▓▓▓▓░░ 20/30│
L14: │ ATK: 15  DEF: 10  NIV: 3                │
L15: │ EXP: 150/300  ARMA: Daga (+8)           │
L16: └───────────────────────────────────────────┘
L21: [W/S] Navegar [A/D] Categoria [SPACE] OK [Q] Salir
```

## Controles

| Tecla | BROWSING | ITEM_ACTIONS |
|-------|----------|--------------|
| **W/S** | Navegar items (circular) | — |
| **A/D** | Cambiar categoría | — |
| **SPACE** | Abrir acciones del item | Ejecutar acción |
| **Q** | Cerrar inventario | Volver a BROWSING |

## Filtrado por categoría

`InventoryUI::buildItemList(ItemCategory cat)` recorre `Jugador::getObjetosInventario()` y filtra por `Objeto::getTipo()`:

| Categoría | `getTipo()` |
|-----------|-------------|
| `ARMAS` | `"Arma"` |
| `POCIONES` | `"Pocion"` |
| `CLAVE` | `"Objeto Clave"` |

## Acciones contextuales (`doAction()`)

| Tipo de item | Acción | Efecto |
|--------------|--------|--------|
| `Arma` | Equipar | Llama `Jugador::equiparArma()`, actualiza ataque |
| `Pocion` | Usar | Llama `Jugador::usarPocion()`, elimina del inventario, reconstruye lista |
| `ObjClave` | Mostrar info | Muestra la descripción del objeto |

## Scroll

- Panel de items: 26x9, contenido visible = listH - 2 = 7 items
- `visibleStart` se ajusta cuando `selectedIndex` supera el rango visible
- Indicadores `▲`/`▼` en la última columna del contenido (`listX + listW - 2`)
- Si la categoría está vacía, muestra "(Sin objetos)" centrado

## Integración desde [[Combate|combate]]

```cpp
// batalla.cpp — case 2 (Inventario):
InventoryUI invUI(*player);
invUI.run();
screenBuffer.forceRedraw();  // restaurar frame de combate
currentState = BattleState::PLAYER_TURN;
```

No usa `suppressCout()`. El ScreenBuffer del inventario escribe directamente a `std::cout` mediante ANSI — es independiente del buffer de combate.

## Integración desde overworld

```cpp
// GameManager.cpp:
void GameManager::mostrarInventario() {
    InventoryUI invUI(jugador);
    invUI.run();
    limpiarPantalla();
    renderMapa();
}
```

Se requiere `std::cin.ignore(numeric_limits<streamsize>::max(), '\n')` antes de abrir porque `std::cin >>` deja `\n` residual.

### Diferencia clave: overlay en combate vs overworld

| Aspecto | En combate | En overworld |
|---------|-----------|--------------|
| Qué hay debajo | Frame de combate (ScreenBuffer) | Mapa renderizado con `std::cout` directo |
| Cómo se restaura | `screenBuffer.forceRedraw()` → BattleSystem redibuja todo | `limpiarPantalla()` + `renderMapa()` desde cero |
| Input fantasma | No hay `\n` residual | Sí, requiere `cin.ignore()` |

## Dependencias

```
Inventario.hpp → Batalla.hpp (ScreenBuffer, colores), Jugador.hpp, Objeto.hpp
Inventario.cpp → Inventario.hpp, Jugador.hpp
batalla.cpp    → ... Inventario.hpp
GameManager.cpp → ... Inventario.hpp
```
