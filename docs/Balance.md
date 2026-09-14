---
creado: 22/07/2026
modificado: 14/09/2026
tipo: Avance
tags:
titulo: Balance
proyecto: "[[LeyendaFacyt]]"
area: Sistemas
estado: Completo
prioridad: 1
dificultad: Media
version: 1.0.0
---
## Links
- Documento Gestor - [[LeyendaFacyt]]
- Sistema de Mapa - [[Mapa]]
- Sistema de Combate - [[Combate]]
## Constantes de Balanceo
Las nuevas variables globales que aplican para diferentes partes del proyecto utilizando `constexpr`

| Nombre Constante        | Valor Inicial |
| ----------------------- | ------------- |
| STAT_BASE_SALUD         | 100           |
| STAT_BASE_ATAQUE        | 15            |
| STAT_BASE_DEFENSA       | 10            |
| STAT_BASE_MANA          | 50            |
| STAT_BASE_POCIONES      | 3             |
| POCION_CURACION_DEFAULT | 30            |
| SALUD_POR_NIVEL         | 50            |
| ATAQUE_POR_NIVEL        | 5             |
| DEFENSA_POR_NIVEL       | 5             |
| COSTO_MAGIA             | 10            |
| MULT_DANO_MAGICO        | 2             |
| BONUS_DANO_NIVEL        | 5             |
| XP_NIVEL_FACTOR_MAX     | 10            |
| EXP_TABLE               | 100→15800 (ver GameBalance.hpp) |
| EXP_CRECIMIENTO_EXTRA   | 400 (repliegue tras la tabla)  |

### Notas de balance (Sprint 3)
- Las stats por nivel son **fijas** (lineales), no cuadráticas: +50 HP / +5 ATK / +5 DEF por nivel (#14).
- La XP necesaria por nivel se deriva de `EXP_TABLE[]` mediante `expRequerida(nivel)` (#15).
- La XP por batalla usa `exp_base (JSON) × nivel_factor × bonus tier`, con `nivel_factor` capado en `XP_NIVEL_FACTOR_MAX` (#13).
