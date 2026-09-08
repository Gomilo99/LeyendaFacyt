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
#include "MapMetadata.hpp"

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

    bool limiteNivelActivo = true; // Se puede desactivar durante pruebas con F8/8
    int nivelMaximoSeccion;
    bool metaCargada;
    MapMetadata metadata;
private:
    Mapa mapa;
    Jugador jugador;
    std::map<std::string, std::shared_ptr<Objeto>> objetos;
    EnemyFactory enemyFactory;
    EncounterManager encounterMgr;
    GameState state;
    int spawnX, spawnY;                     ///< Posición inicial en el mapa original
    int nivelActual;
    bool jefeDerrotado;
    bool haGanadoFinal;

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
    bool cargarNivel(int nivel);
    void cargarMetadata(int nivel);
    const ZoneMetadata* zonaActual() const;
};

#endif
