#include <iostream>
#include <fstream>
#include "../lib/GameManager.hpp"
#include "../lib/DataManager.hpp"
#include "../lib/batalla.hpp"
#include "../lib/config.hpp"

GameManager::GameManager()
    : jugador("Heroe"), state(GameState::MAIN_MENU)
{
    objetos = DataManager::cargarObjetos();
    if (objetos.empty()) {
        std::cerr << "No se pudieron cargar los objetos desde el archivo JSON.\n";
    }

    enemyFactory.cargarDesdeJSON(Config::enemigosPath(), objetos);

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

void GameManager::mostrarMenuPrincipal() {
    limpiarPantalla();
    std::cout << "========================================\n";
    std::cout << "       LEYENDA DEL CAMPUS\n";
    std::cout << "       La Leyenda FACYT\n";
    std::cout << "========================================\n";
    std::cout << "\n  Un RPG de mazmorras en ASCII\n";
    std::cout << "\n  Explora, lucha contra criaturas\n";
    std::cout << "  y encuentra la llave magica!\n";
    std::cout << "\n  [WASD] Mover  [I] Inventario  [Q] Salir\n";
    std::cout << "\n  Presiona Enter para comenzar...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
    state = GameState::OVERWORLD;
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
        char tile = mapa.getTile(nuevoX, nuevoY);
        handleTile(tile);

        if (jugador.estaVivo() && !jugador.getHaGanado()
            && tile != 'B' && tile != 'K' && tile != 'H') {
            encounterMgr.registrarPaso();
            if (encounterMgr.verificarEncuentro()) {
                iniciarCombate();
            }
        }
    }
}

void GameManager::handleTile(char tile) {
    if (tile == 'B'){
        iniciarCombateJefe();
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
    Enemigo enemigo = enemyFactory.crearEnemigo(jugador.getNivel());
    batalla(jugador, enemigo);
}

void GameManager::iniciarCombateJefe() {
    if (enemyFactory.hayJefe(jugador.getNivel())) {
        Enemigo jefe = enemyFactory.crearJefe(jugador.getNivel());
        batalla(jugador, jefe);
    } else {
        std::cout << "Aun no hay un jefe para tu nivel...\n";
        std::cout << "Un enemigo aparece de todas formas!\n";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        iniciarCombate();
    }
}

void GameManager::run() {
    state = GameState::MAIN_MENU;

    while (true) {
        switch (state) {
            case GameState::MAIN_MENU:
                mostrarMenuPrincipal();
                break;

            case GameState::OVERWORLD:
                while (jugador.estaVivo() && !jugador.getHaGanado()){
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
                    case 'q': case 'Q': return;
                    default: continue;
                    }

                    moverJugador(dx, dy);
                }

                if (jugador.getHaGanado()) {
                    std::cout << "\nMuchas gracias por Jugar :)" << std::endl;
                    return;
                }
                state = GameState::GAME_OVER;
                break;

            case GameState::GAME_OVER:
                std::cout << "\n\nGAME OVER" << std::endl;
                return;
        }
    }
}
