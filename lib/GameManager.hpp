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
    Mapa mapa;
    Jugador jugador;
    std::map<std::string, std::shared_ptr<Objeto>> objetos;
    EnemyFactory enemyFactory;
    EncounterManager encounterMgr;
    GameState state;
    int spawnX, spawnY;                     ///< Posición inicial en el mapa original

    void mostrarMenuPrincipal();
    void inicializarNuevaPartida();
    bool cargarPartidaExistente();
    void guardarPartida();
    void renderMapa();
    void moverJugador(int dx, int dy);
    void handleTile(char tile);
    void mostrarInventario();
    void iniciarCombate();
    void iniciarCombateJefe();
};

#endif
