#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "../lib/GameManager.hpp"
#include "../lib/DataManager.hpp"
#include "../lib/CacheManager.hpp"
#include "../lib/Batalla.hpp"
#include "../lib/Config.hpp"
#include "../lib/ArtLoader.hpp"
#include "../lib/Inventario.hpp"

/**
 * Constructor del motor del juego.
 *
 * Solo carga datos estaticos (objetos, enemigos) y encuentra
 * el spawn point del mapa original. La eleccion entre
 * Nueva Partida / Continuar se hace en mostrarMenuPrincipal().
 */
GameManager::GameManager()
    : jugador("Heroe"), state(GameState::MAIN_MENU), spawnX(1), spawnY(1)
{
    objetos = DataManager::cargarObjetos();
    if (objetos.empty()) {
        std::cerr << "No se pudieron cargar los objetos desde el archivo JSON.\n";
    }

    enemyFactory.cargarDesdeJSON(Config::enemigosPath(), objetos);

    // Buscar spawn point 'P' del mapa original
    Mapa mapaTemp;
    if (mapaTemp.cargar(Config::mapaPath())){
        for (int y = 0; y < mapaTemp.getAlto(); y++){
            for (int x = 0; x < mapaTemp.getAncho(); x++){
                if (mapaTemp.getTile(x, y) == 'P'){
                    spawnX = x;
                    spawnY = y;
                }
            }
        }
    } else {
        std::cerr << "No se pudo cargar el mapa original.\n";
    }
}

/**
 * Pantalla de titulo con menu de 3 opciones:
 * 1. Nueva Partida — borra cache, crea partida fresca
 * 2. Continuar    — carga desde cache si existe
 * 3. Salir        — cierra el juego
 */
void GameManager::mostrarMenuPrincipal() {
    while (true) {
        limpiarPantalla();
        auto arte = ArtLoader::cargarArte("assets/title.txt");

        for (const auto& linea : arte)
            std::cout << "\033[93m" << linea << "\033[0m\n";

        std::cout << "\033[36m\n  1. Nueva Partida\033[0m\n";
        std::cout << "\033[36m  2. Continuar\033[0m\n";
        std::cout << "\033[36m  3. Salir\033[0m\n";
        std::cout << "\033[92m\n  Elige una opcion: \033[0m";

        int opcion;
        std::cin >> opcion;

        switch (opcion) {
            case 1:
                inicializarNuevaPartida();
                return;
            case 2:
                if (cargarPartidaExistente())
                    return;
                std::cout << "\033[91mNo hay partida guardada.\033[0m\n";
                std::cout << "Presiona Enter para volver al menu...";
                limpiarBuffer();
                std::cin.get();
                break;
            case 3:
                state = GameState::GAME_OVER;
                return;
            default:
                continue;
        }
    }
}

/**
 * Inicializa una partida nueva:
 * 1. Limpia cache/
 * 2. Carga mapa fresco del archivo original
 * 3. Crea heroe con estadisticas base
 * 4. Equipa Espada Gallo
 * 5. Pide nombre al jugador
 * 6. Guarda todo en cache
 */
void GameManager::inicializarNuevaPartida() {
    CacheManager::limpiar();

    if (!mapa.cargar(Config::mapaPath())){
        std::cerr << "No se pudo cargar el mapa.\n";
        return;
    }

    jugador = Jugador("Heroe");
    jugador.setPos(spawnX, spawnY);

    auto itEspada = objetos.find("Espada Gallo");
    if (itEspada != objetos.end()) {
        jugador.equiparArma(std::dynamic_pointer_cast<Arma>(itEspada->second), true);
    }

    std::cout << "\033[36mIngresa tu nombre (Enter para 'Heroe'): \033[0m";
    std::string nombreInput;
    limpiarBuffer();
    std::getline(std::cin, nombreInput);
    if (!nombreInput.empty())
        jugador.setNombre(nombreInput);

    CacheManager::crearPartida(mapa, jugador);
    state = GameState::OVERWORLD;
}

/**
 * Carga una partida existente desde cache/.
 * Retorna true si se pudo cargar correctamente.
 */
bool GameManager::cargarPartidaExistente() {
    if (!CacheManager::existePartida())
        return false;

    if (!CacheManager::cargarMapa(mapa)) {
        std::cerr << "Error al cargar el mapa guardado.\n";
        return false;
    }

    jugador = CacheManager::cargarHeroe(objetos);
    state = GameState::OVERWORLD;
    return true;
}

/**
 * Guarda el estado actual del heroe y el mapa en cache/.
 */
void GameManager::guardarPartida() {
    CacheManager::guardarHeroe(jugador);
    CacheManager::guardarMapa(mapa);
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
 * Procesa tiles especiales del mapa y persiste los cambios en cache.
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
            CacheManager::guardarMapa(mapa);
        }
    }
    if (tile == 'K'){
        std::cout << "Has encontrado la llave magica!\n";
        jugador.setHaGanado(true);
    }
    if (tile == 'H'){
        jugador.usarPocion();
        mapa.setTile(jugador.getPosX(), jugador.getPosY(), '.');
        CacheManager::guardarMapa(mapa);
    }
}

void GameManager::mostrarInventario() {
    InventoryUI invUI(jugador);
    invUI.run();
    // Al salir, la pantalla tiene basura del inventario
    // asi que limpias y re-renderizas el mapa
    limpiarPantalla();
    renderMapa();
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
 * Bucle principal del juego con maquina de estados explicita.
 *
 * MAIN_MENU → OVERWORLD → (BATTLE anidado) → OVERWORLD o GAME_OVER
 *
 * OVERWORLD:
 *   - WASD para mover
 *   - I para inventario
 *   - Q guarda la partida y sale
 *   - Al morir → GAME_OVER
 *   - Al ganar → mensaje de victoria
 */
void GameManager::run() {
    state = GameState::MAIN_MENU;

    while (true) {
        switch (state) {
            case GameState::MAIN_MENU:
                mostrarMenuPrincipal();
                if (state == GameState::GAME_OVER)
                    return;
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
                    case 'q': case 'Q':
                        guardarPartida();
                        return;
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