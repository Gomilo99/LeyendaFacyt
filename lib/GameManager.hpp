#ifndef GAME_MANAGER_HPP
#define GAME_MANAGER_HPP

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
    enum class GameState {
        MAIN_MENU,
        OVERWORLD,
        GAME_OVER
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

    void mostrarMenuPrincipal();
    void renderMapa();
    void moverJugador(int dx, int dy);
    void handleTile(char tile);
    void mostrarInventario();
    void iniciarCombate();
    void iniciarCombateJefe();
};

#endif
