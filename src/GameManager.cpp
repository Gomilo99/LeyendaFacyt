#include <iostream>
#include <fstream>
#include "../lib/GameManager.hpp"
#include "../lib/DataManager.hpp"
#include "../lib/Batalla.hpp"
#include "../lib/Config.hpp"
#include "../lib/ArtLoader.hpp"

/**
 * Constructor del motor del juego.
 *
 * Flujo de inicialización:
 * 1. Carga objetos desde objetos.json
 * 2. Carga plantillas de enemigos desde enemigos.json (EnemyFactory)
 * 3. Carga el mapa desde mapas/nivel1.txt
 * 4. Si existe heroe.json, restaura la partida guardada
 * 5. Busca el tile 'P' en el mapa para colocar al jugador
 * 6. Equipa la "Espada Gallo" por defecto si existe en el JSON
 */
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

/**
 * Renderiza la pantalla de título principal.
 * Muestra el nombre del juego, instrucciones básicas
 * y espera a que el jugador presione Enter para comenzar.
 * Luego transiciona al estado OVERWORLD.
 */
void GameManager::mostrarMenuPrincipal() {
    //limpiarPantalla();
    ScreenBuffer menuBuf;
    ScreenBuffer::hideCursor();

    auto arte = ArtLoader::cargarArte("assets/title.txt");

    menuBuf.clear();
    menuBuf.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 1, COL_CYAN);
    ArtLoader::dibujarArteCentrado(menuBuf, arte, COL_BYELLOW);
    // Instrucciones al pie
    menuBuf.drawString(10, 18, "Explora, lucha y encuentra la llave magica!", COL_CYAN);
    menuBuf.drawString(12, 20, "[WASD] Mover [I] Inventario [Q] Salir", COL_WHITE);
    menuBuf.drawString(14, 21, "Presiona Enter para comenzar...", COL_BGREEN);
    menuBuf.render();

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
    ScreenBuffer::showCursor();

    state = GameState::OVERWORLD;
}

/**
 * Renderiza el mapa en vista top-down.
 * Dibuja la cuadrícula completa, reemplazando la posición
 * del jugador con '@'.
 */
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

/**
 * Mueve al jugador si el tile destino es transitable.
 *
 * Flujo:
 * 1. Calcula nueva posición (nuevoX, nuevoY)
 * 2. Si es transitable, actualiza la posición
 * 3. Procesa el tile pisado (B = boss, K = victoria, H = poción)
 * 4. Si no hubo evento especial y el jugador sigue vivo:
 *    - Registra el paso en EncounterManager
 *    - Verifica si debe ocurrir un encuentro aleatorio
 */
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

/**
 * Procesa tiles especiales del mapa.
 *
 * B: Inicia combate contra el jefe del nivel
 * K: Marca victoria (jugador encontró la llave)
 * H: Usa una poción y elimina el tile del mapa
 */
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

/**
 * Inicia un combate contra un enemigo aleatorio.
 * EnemyFactory selecciona una plantilla mediante peso ponderado
 * y crea una instancia de Enemigo lista para batalla().
 */
void GameManager::iniciarCombate() {
    Enemigo enemigo = enemyFactory.crearEnemigo(jugador.getNivel());
    batalla(jugador, enemigo);
}

/**
 * Inicia un combate contra el jefe del nivel actual.
 * Si no hay jefe definido para el nivel del jugador,
 * muestra un mensaje y cae en un combate aleatorio normal.
 */
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

/**
 * Bucle principal del juego con máquina de estados explícita.
 *
 * MAIN_MENU → OVERWORLD → (BATTLE anidado) → OVERWORLD o GAME_OVER
 *
 * El estado BATTLE se maneja dentro de iniciarCombate()/iniciarCombateJefe()
 * que llaman a batalla() de forma síncrona; al terminar el combate
 * se retorna al bucle OVERWORLD.
 */
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