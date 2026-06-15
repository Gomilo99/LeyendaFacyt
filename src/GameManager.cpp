#include <iostream>
#include <fstream>
#include "../lib/GameManager.hpp"
#include "../lib/DataManager.hpp"
#include "../lib/batalla.hpp"
#include "../lib/config.hpp"

GameManager::GameManager()
    : jugador("Heroe")
{
    objetos = DataManager::cargarObjetos();
    if (objetos.empty()) {
        std::cerr << "No se pudieron cargar los objetos desde el archivo JSON.\n";
    }

    enemigos = DataManager::cargarEnemigos(objetos);
    if (enemigos.empty()) {
        std::cerr << "No se pudieron cargar los enemigos desde el archivo JSON.\n";
    }

    if(!mapa.cargar(Config::mapaPath())){
        std::cerr << "No se pudo cargar el mapa. ERROR: Archivo no encontrado.\n";
    }

    std::ifstream heroeFile(Config::heroePath());
    if(heroeFile.is_open()) {
        jugador = DataManager::cargarHeroe();
    }
    heroeFile.close();

    for(int y = 0; y < mapa.getAlto(); y++){
        for(int x = 0; x < mapa.getAncho(); x++){
            if(mapa.getTile(x, y) == 'P'){
                jugador.setPos(x, y);
            }
        }
    }

    auto itEspada = objetos.find("Espada Gallo");
    if (itEspada != objetos.end()) {
        jugador.equiparArma(std::dynamic_pointer_cast<Arma>(itEspada->second));
    }
}

void GameManager::renderMapa() {
    for (int y = 0; y < mapa.getAlto(); y++){
        for (int x = 0; x < mapa.getAncho(); x++){
            if (x == jugador.getPosX() && y == jugador.getPosY()){
                std::cout << '@';
            } else {
                std::cout << mapa.getTile(x, y);
            }
        }
        std::cout << std::endl;
    }
}

void GameManager::moverJugador(int dx, int dy) {
    int nuevoX = jugador.getPosX() + dx;
    int nuevoY = jugador.getPosY() + dy;

    if (mapa.esTransitable(nuevoX, nuevoY)){
        jugador.setPos(nuevoX, nuevoY);
        handleTile(mapa.getTile(nuevoX, nuevoY));
    }
}

void GameManager::handleTile(char tile) {
    if (tile == 'E'){
        iniciarCombate();
        if (jugador.estaVivo()){
            mapa.setTile(jugador.getPosX(), jugador.getPosY(), '.');
        }
    }
    if (tile == 'K'){
        std::cout << "Has encontrado la llave magica!\n";
        jugador.setHaGanado(true);
    }
    if (tile == 'H'){
        jugador.usarPocion();
        mapa.setTile(jugador.getPosX(), jugador.getPosY(), '.');
    }
}

void GameManager::mostrarInventario() {
    jugador.mostrarInventario();
}

void GameManager::iniciarCombate() {
    auto enemigoSelec = DataManager::generarEnemigoPorNivel(enemigos, jugador.getNivel());
    Enemigo enemigo(*enemigoSelec);
    batalla(jugador, enemigo);
}

void GameManager::run() {
    bool explorando = true;
    while (jugador.estaVivo() && !jugador.getHaGanado() && explorando){
        limpiarPantalla();
        renderMapa();

        std::cout << "\nWASD para mover, I inventario, Q salir: ";
        char input;
        std::cin >> input;
        int dx = 0, dy = 0;
        switch (input) {
        case 'w': case 'W': dy = -1; break;
        case 's': case 'S': dy = 1; break;
        case 'a': case 'A': dx = -1; break;
        case 'd': case 'D': dx = 1; break;
        case 'i': case 'I': mostrarInventario(); continue;
        case 'q': case 'Q': explorando = false; continue;
        default: continue;
        }

        moverJugador(dx, dy);
    }

    if (jugador.getHaGanado()) {
        std::cout << "\nMuchas gracias por Jugar :)" << std::endl;
    } else {
        std::cout << "\n\nGAME OVER";
    }
}
