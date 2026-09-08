#ifndef GAME_BALANCE_HPP
#define GAME_BALANCE_HPP

// === Stats base del heroe (constructor por defecto) ===
constexpr int STAT_BASE_SALUD       = 100;  
constexpr int STAT_BASE_ATAQUE      = 15;   
constexpr int STAT_BASE_DEFENSA     = 10;   
constexpr int STAT_BASE_MANA        = 50;   
constexpr int STAT_BASE_POCIONES    = 3;    

// === Pociones ===
constexpr int POCION_CURACION_DEFAULT = 30;

// === Sistema de nivelación ===
constexpr int SALUD_POR_NIVEL       = 50;   // HP_max += SAUD_POR_NIVEL * (nivel+1)
constexpr int ATAQUE_POR_NIVEL      = 5;    // ataque += ATAQUE_POR_NIVEL * (nivel+1)
constexpr int DEFENSA_POR_NIVEL     = 5;    // defensa += DEFENSA_POR_NIVEL * (nivel+1)
constexpr int EXP_INCREMENTO        = 200;  // expNecesaria += EXP_INCREMENTO por nivel
constexpr int EXP_NIVEL_3           = 700;  // caso especial para nivel 3
constexpr int EXP_UMBRAL_BASE       = 100;  // expNecesaria inicial (Jugador.hpp:19)

// === Magia ===
constexpr int COSTO_MAGIA       = 10;   // MP minimo para lanzar
constexpr int MULT_DANO_MAGICO  = 2;    // ataque * MULT_DANO_MAGICO
constexpr int BONUS_DANO_NIVEL  = 5;    // nivel * BONUS_DANO_NIVEL

// === XP por batalla ===
constexpr int XP_BASE    = 50;   // experiencia base
constexpr float XP_MULT_JEFE = 4.0f;
constexpr float XP_MULT_CAMPEON = 2.1f;

constexpr float XP_MULT_ELITE = 1.8f;
constexpr float XP_MULT_AVANZADO = 1.35f;



#endif