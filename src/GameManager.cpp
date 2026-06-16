#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
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
        jugador.equiparArma(std::dynamic_pointer_cast<Arma>(itEspada->second), true);
    }
}

/**
 * Renderiza la pantalla de título principal.
 * Muestra el nombre del juego, instrucciones básicas
 * y espera a que el jugador presione Enter para comenzar.
 * Luego transiciona al estado OVERWORLD.
 */
void GameManager::mostrarMenuPrincipal() {
    limpiarPantalla();
    auto arte = ArtLoader::cargarArte("assets/title.txt");

    for (const auto& linea : arte)
        std::cout << "\033[93m" << linea << "\033[0m\n";

    std::cout << "\033[36m\n  Explora, lucha contra criaturas\033[0m\n";
    std::cout << "\033[36m  y encuentra la llave magica!\033[0m\n";
    std::cout << "\033[37m\n  [WASD] Mover  [I] Inventario  [Q] Salir\033[0m\n";
    std::cout << "\033[92m\n  Presiona Enter para comenzar...\033[0m";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    if (jugador.getNombre() == "Heroe") {
        std::cout << "\033[36mIngresa tu nombre (Enter para 'Heroe'): \033[0m";
        std::string nombreInput;
        std::getline(std::cin, nombreInput);
        if (!nombreInput.empty())
            jugador.setNombre(nombreInput);
    }
    state = GameState::OVERWORLD;
}

/**
 * Renderiza el mapa en vista top-down con el HUD del jugador a la derecha.
 * Los tiles del mapa tienen color según su tipo.
 * HUD muestra: nombre, nivel, EXP, HP, MP, arma, pociones.
 */
void GameManager::renderMapa() {
    int anchoMapa = mapa.getAncho();
    int altoMapa = mapa.getAlto();

    int hpPct = (jugador.getSaludMaxima() > 0)
        ? (jugador.getSalud() * 100 / jugador.getSaludMaxima()) : 0;
    std::string hpColor = (hpPct > 50) ? "32" : (hpPct > 25) ? "33" : "31";

    int barW = 10;
    int hpFill = (jugador.getSaludMaxima() > 0)
        ? (jugador.getSalud() * barW / jugador.getSaludMaxima()) : 0;
    int mpFill = (jugador.getManaMaxima() > 0)
        ? (jugador.getMana() * barW / jugador.getManaMaxima()) : 0;

    std::string hpBar = std::string(hpFill, '#') + std::string(barW - hpFill, '.');
    std::string mpBar = std::string(mpFill, '#') + std::string(barW - mpFill, '.');

    std::vector<std::string> hud;
    hud.push_back("\033[36m+-----------------------+\033[0m");
    hud.push_back("\033[93m|  " + jugador.getNombre() + "\033[0m");
    hud.push_back("\033[97m|  Nv: " + std::to_string(jugador.getNivel())
        + "  Exp: " + std::to_string(jugador.getExperiencia())
        + "/" + std::to_string(jugador.getExperienciaNecesaria()) + "\033[0m");
    hud.push_back("\033[" + hpColor + "m|  HP: "
        + std::to_string(jugador.getSalud()) + "/"
        + std::to_string(jugador.getSaludMaxima()) + " " + hpBar + "\033[0m");
    hud.push_back("\033[94m|  MP: "
        + std::to_string(jugador.getMana()) + "/"
        + std::to_string(jugador.getManaMaxima()) + " " + mpBar + "\033[0m");
    hud.push_back("\033[97m|  Arma: " + jugador.getArmaNombre() + "\033[0m");
    hud.push_back("\033[97m|  Pociones: " + std::to_string(jugador.getPociones()) + "\033[0m");
    hud.push_back("\033[36m+-----------------------+\033[0m");

    int altoHud = (int)hud.size();
    int altoTotal = std::max(altoMapa, altoHud);

    for (int y = 0; y < altoTotal; y++) {
        if (y < altoMapa) {
            for (int x = 0; x < anchoMapa; x++) {
                if (x == jugador.getPosX() && y == jugador.getPosY()) {
                    std::cout << "\033[93m@\033[0m";
                } else {
                    char t = mapa.getTile(x, y);
                    switch (t) {
                        case '#': std::cout << "\033[90m#\033[0m"; break;
                        case '.': std::cout << "\033[32m.\033[0m"; break;
                        case 'K': std::cout << "\033[93mK\033[0m"; break;
                        case 'B': std::cout << "\033[91mB\033[0m"; break;
                        case 'H': std::cout << "\033[92mH\033[0m"; break;
                        default:  std::cout << t;
                    }
                }
            }
        } else {
            for (int x = 0; x < anchoMapa; x++) std::cout << ' ';
        }

        std::cout << "  ";
        if (y < altoHud) std::cout << hud[y];
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