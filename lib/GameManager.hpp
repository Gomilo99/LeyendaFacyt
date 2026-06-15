#ifndef GAME_MANAGER_HPP
#define GAME_MANAGER_HPP

#include <map>
#include <memory>
#include <string>
#include "mapa.hpp"
#include "jugador.hpp"
#include "objeto.hpp"
#include "enemigo.hpp"

class GameManager {
    Mapa mapa;
    Jugador jugador;
    std::map<std::string, std::shared_ptr<Objeto>> objetos;
    std::map<std::string, std::shared_ptr<Enemigo>> enemigos;

    void renderMapa();
    void moverJugador(int dx, int dy);
    void handleTile(char tile);
    void mostrarInventario();
    void iniciarCombate();

public:
    GameManager();
    void run();
};

#endif
