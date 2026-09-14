#ifndef GAME_BALANCE_HPP
#define GAME_BALANCE_HPP

#include <algorithm>

// === Stats base del heroe (constructor por defecto) ===
constexpr int STAT_BASE_SALUD       = 100;  
constexpr int STAT_BASE_ATAQUE      = 15;   
constexpr int STAT_BASE_DEFENSA     = 10;   
constexpr int STAT_BASE_MANA        = 50;   
constexpr int STAT_BASE_POCIONES    = 3;    

// === Pociones ===
constexpr int POCION_CURACION_DEFAULT = 30;

// === Sistema de nivelación ===
// Crecimiento de stats por nivel: FIJOS (lineal). Antes se usaba *(nivel+1),
// lo que escalaba de forma cuadrática y rompía el balance. Ver auditoría #14.
constexpr int SALUD_POR_NIVEL       = 50;   // +50 HP al subir de nivel
constexpr int ATAQUE_POR_NIVEL      = 5;    // +5 ATAQUE
constexpr int DEFENSA_POR_NIVEL     = 5;    // +5 DEFENSA

// Curva de nivelación expresada como tabla (auditoría #15):
// EXP necesaria para pasar del nivel N al N+1 (índice 0 = nivel 1).
// La tabla es la ÚNICA fuente de verdad de la curva de nivel.
constexpr int EXP_TABLE[] = {
    100, 200, 350, 550, 800, 1100, 1500, 2000,
    2600, 3300, 4100, 5000, 6000, 7100, 8300,
    9600, 11000, 12500, 14100, 15800
};
constexpr int EXP_TABLE_SIZE = sizeof(EXP_TABLE) / sizeof(EXP_TABLE[0]);
constexpr int EXP_CRECIMIENTO_EXTRA = 400;  // repliegue lineal tras superar la tabla

// XP requerida para el nivel dado. Fuera de la tabla crece lineal con
// EXP_CRECIMIENTO_EXTRA para que el juego nunca se atasque.
inline int expRequerida(int nivel) {
    int idx = std::max(1, nivel) - 1;
    if (idx >= EXP_TABLE_SIZE)
        return EXP_TABLE[EXP_TABLE_SIZE - 1] + (idx - EXP_TABLE_SIZE + 1) * EXP_CRECIMIENTO_EXTRA;
    return EXP_TABLE[idx];
}

// === Magia ===
constexpr int COSTO_MAGIA       = 10;   // MP minimo para lanzar
constexpr int MULT_DANO_MAGICO  = 2;    // ataque * MULT_DANO_MAGICO
constexpr int BONUS_DANO_NIVEL  = 5;    // nivel * BONUS_DANO_NIVEL

// === XP por batalla ===
// Fórmula canónica (auditoría #13):
//   exp_base (del JSON del enemigo) × nivel_factor × bonus de tier
// nivel_factor crece linealmente con el nivel del enemigo pero con tope suave,
// para que los enemigos de zonas altas o los jefes no disparen la curva.
constexpr int XP_NIVEL_FACTOR_MAX = 10;
inline int xpNivelFactor(int nivel) {
    return std::clamp(nivel, 1, XP_NIVEL_FACTOR_MAX);
}



#endif