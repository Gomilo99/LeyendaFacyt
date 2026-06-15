#ifndef GAME_MANAGER_HPP
#define GAME_MANAGER_HPP

/**
 * @file GameManager.hpp
 * Motor principal del juego con máquina de estados explícita.
 *
 * Estados (GameState):
 *   MAIN_MENU → OVERWORLD → (BATTLE, anidado) → OVERWORLD o GAME_OVER
 *
 * Integra:
 * - EnemyFactory:    carga y crea enemigos desde JSON
 * - EncounterManager: decide cuándo ocurren encuentros aleatorios
 * - Mapa:            grid de tiles con obstáculos y eventos
 * - Jugador:         personaje controlado por el usuario
 */

#include <map>
#include <memory>
#include <string>
#include "mapa.hpp"
#include "jugador.hpp"
#include "objeto.hpp"
#include "enemyFactory.hpp"
#include "encounterManager.hpp"

class GameManager {
public:
    /// Estados de la máquina de estados del juego
    enum class GameState {
        MAIN_MENU,  ///< Pantalla de título, espera Enter
        OVERWORLD,  ///< Exploración del mapa con encuentros aleatorios
        GAME_OVER   ///< Jugador murió, fin de la partida
    };

    GameManager();
    void run();

private:
    Mapa mapa;                              ///< Grid del nivel actual
    Jugador jugador;                        ///< Personaje del jugador
    std::map<std::string, std::shared_ptr<Objeto>> objetos; ///< Catálogo de objetos del JSON
    EnemyFactory enemyFactory;              ///< Fábrica de enemigos (lectura JSON + creación)
    EncounterManager encounterMgr;          ///< Gestor de encuentros aleatorios
    GameState state;                        ///< Estado actual de la máquina

    void mostrarMenuPrincipal();            ///< Renderiza el título y espera Enter
    void renderMapa();                      ///< Dibuja el mapa top-down con @ para el jugador
    void moverJugador(int dx, int dy);      ///< Mueve al jugador y chequea eventos
    void handleTile(char tile);             ///< Procesa el tile pisado (B, K, H)
    void mostrarInventario();               ///< Delega en Jugador::mostrarInventario
    void iniciarCombate();                  ///< Crea enemigo aleatorio y entra en batalla
    void iniciarCombateJefe();              ///< Busca el jefe del nivel y entra en batalla
};

#endif
