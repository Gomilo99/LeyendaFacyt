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
#include "../lib/Platform.hpp"

/**
 * Constructor del motor del juego.
 *
 * Solo carga datos estaticos (objetos, enemigos) y encuentra
 * el spawn point del mapa original. La eleccion entre
 * Nueva Partida / Continuar se hace en mostrarMenuPrincipal().
 */
GameManager::GameManager()
    : jugador("Heroe"), state(GameState::MAIN_MENU), spawnX(1), spawnY(1),
    nivelActual(1), jefeDerrotado(false), haGanadoFinal(false)
{
    inicializarTileRegistry();
    objetos = DataManager::cargarObjetos();
    if (objetos.empty()) {
        std::cerr << "No se pudieron cargar los objetos desde el archivo JSON.\n";
    }

    enemyFactory.cargarDesdeJSON(Config::enemigosPath(), objetos);
    cargarMetadata(1);

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

void GameManager::cargarMetadata(int nivel) {
    metaCargada = metadata.load(Config::mapaMetaPath(nivel));
    if (!metaCargada) {
        metadata = MapMetadata();
        metadata.levelCap = 2;
    }
    nivelMaximoSeccion = metadata.levelCap;
    jugador.setNivelMaximoPermitido(nivelMaximoSeccion);
    encounterMgr.configurar(metadata.encounterBase, metadata.encounterMultiplier,
                            metadata.encounterGrowthCap, metadata.encounterGraceSteps);
}

const ZoneMetadata* GameManager::zonaActual() const {
    return metaCargada
        ? metadata.zoneAt(mapa.getTile(jugador.getPosX(), jugador.getPosY()))
        : nullptr;
}

const ZoneMetadata* GameManager::zonaJefeActual() const {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            char terrainTile = mapa.getTile(jugador.getPosX() + dx, jugador.getPosY() + dy);
            for (const auto& zone : metadata.zones) {
                if (zone.tile == terrainTile && !zone.bossId.empty()) return &zone;
            }
        }
    }
    return nullptr;
}

/**
 * Pantalla de titulo con menu de 3 opciones:
 * 1. Nueva Partida — borra cache, crea partida fresca
 * 2. Continuar    — carga desde cache si existe
 * 3. Salir        — cierra el juego
 */
void GameManager::mostrarMenuPrincipal() {
    while (true) {
        Platform::clearScreen();
        auto arte = ArtLoader::cargarArte("assets/title.txt");

        for (const auto& linea : arte)
            std::cout << "\033[93m" << linea << "\033[0m\n";

        std::cout << "\033[36m\n  1. Nueva Partida\033[0m\n";
        std::cout << "\033[36m  2. Continuar\033[0m\n";
        std::cout << "\033[36m  3. Salir\033[0m\n";
        std::cout << "\033[92m\n  Elige una opcion: \033[0m";

        int opcion = 0;
        char c = Platform::getKey();
        if (c >= '1' && c <= '3') opcion = c - '0';

        switch (opcion) {
            case 1:
                inicializarNuevaPartida();
                return;
            case 2:
                if (cargarPartidaExistente())
                    return;
                std::cout << "\033[91mNo hay partida guardada.\033[0m\n";
                std::cout << "Presiona Enter para volver al menu...";
                Platform::clearInputBuffer();
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

nivelActual = 1;
    jefeDerrotado = false;
    haGanadoFinal = false;
    cargarMetadata(1);
    encounterMgr.configurarPorNivel(1);

    // Carga de personaje y spawn
    jugador = Jugador("Heroe");
    jugador.setPos(spawnX, spawnY);

    auto itEspada = objetos.find("Espada Gallo");
    if (itEspada != objetos.end()) {
        jugador.equiparArma(std::dynamic_pointer_cast<Arma>(itEspada->second), true);
    }

    std::cout << "\033[36mIngresa tu nombre (Enter para 'Heroe'): \033[0m";
    std::string nombreInput;
    Platform::echoOn();
    std::getline(std::cin, nombreInput);
    Platform::echoOff();
    if (!nombreInput.empty())
        jugador.setNombre(nombreInput);

    CacheManager::crearPartida(mapa, jugador);
    CacheManager::guardarEstado({nivelActual, jefeDerrotado, haGanadoFinal});
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

    //  === Carga de jugador ===
    jugador = CacheManager::cargarHeroe(objetos);

CacheManager::EstadoPartida estado;
    if (CacheManager::cargarEstado(estado)) {
        nivelActual = estado.nivelActual;
        jefeDerrotado = estado.jefeDerrotado;
        haGanadoFinal = estado.haGanadoFinal;
    } else {
        nivelActual = jugador.getNivelActual();
        jefeDerrotado = false;
        haGanadoFinal = jugador.getHaGanado();
        bool hayJefeEnMapa = false;
        for (int y = 0; y < mapa.getAlto(); y++) {
            for (int x = 0; x < mapa.getAncho(); x++) {
                if (mapa.getTile(x, y) == 'B') {
                    hayJefeEnMapa = true;
                    break;
                }
            }
        }
        jefeDerrotado = !hayJefeEnMapa;
    }
    jugador.setNivelActual(nivelActual);
    cargarMetadata(nivelActual);
    encounterMgr.configurarPorNivel(nivelActual);
    if (haGanadoFinal) jugador.setHaGanado(true);
    state = GameState::OVERWORLD;
    return true;
}

/**
 * Guarda el estado actual del heroe y el mapa en cache/.
 */
void GameManager::guardarPartida() {
    CacheManager::guardarPartida(mapa, jugador);
    CacheManager::guardarEstado({nivelActual, jefeDerrotado, haGanadoFinal});
}

/**
 * Renderiza el mapa en vista top-down con el HUD del jugador a la derecha.
 * Los tiles del mapa tienen color según su tipo.
 * HUD muestra: nombre, nivel, EXP, HP, MP, arma, pociones.
 */
void GameManager::renderMapa() {
    overworldRenderer.renderAll(jugador, mapa, metadata, zonaActual(),
                                nivelMaximoSeccion, limiteNivelActivo);
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
        int viejoX = jugador.getPosX();
        int viejoY = jugador.getPosY();
        jugador.setPos(nuevoX, nuevoY);
        if (mapa.getTile(viejoX, viejoY) == 'P'){
            mapa.setTile(viejoX, viejoY, 's');
            CacheManager::guardarMapa(mapa);
        }
        char tile = mapa.getTile(nuevoX, nuevoY);
        handleTile(tile);

        if (jugador.estaVivo() && !jugador.getHaGanado()
            && tile != 'B' && tile != 'K' && tile != 'H' &&
            tile != 'h' && tile != 'G') {
            const ZoneMetadata* zone = zonaActual();
            encounterMgr.configurar(metadata.encounterBase, metadata.encounterMultiplier *
                                    (zone ? zone->encounterMultiplier : 1.0f),
                                    metadata.encounterGrowthCap, metadata.encounterGraceSteps,
                                    zone ? zone->safe : false);
            encounterMgr.ajustarPorNivel(jugador.getNivel(), zone ? zone->nivelSugerido : nivelActual);
            encounterMgr.registrarPaso();
            if (encounterMgr.verificarEncuentro()) {
                iniciarCombate();
            }
        }
    }
}

/**
 * Inicializa los manejadores de eventos para cada tipo de tile especial.
 *
 * B: Inicia el combate contra el jefe de la zona y habilita K al ganar
 * K: Carga el siguiente nivel si el jefe fue derrotado
 * H/h/G: Curación porcentual y restauración del terreno base del nivel
 */
void GameManager::inicializarTileRegistry() {
    tileRegistry.registrarManejador('B', [this](GameManager&, int, int) {
        const ZoneMetadata* zone = zonaJefeActual();
        if (!zone || zone->bossId.empty()) {
            std::cerr << "Error de configuracion: el tile B no tiene jefe asignado.\n";
            return;
        }
        iniciarCombateJefe();
        if (jugador.estaVivo()){
            jefeDerrotado = true;
            mapa.setTile(jugador.getPosX(), jugador.getPosY(),
                        zone->restoreTile != '\0' ? zone->restoreTile :
                        (zone->tile != '\0' ? zone->tile : '.'));
            guardarPartida();
        }
    });

    tileRegistry.registrarManejador('K', [this](GameManager&, int, int) {
        if (!jefeDerrotado) {
            std::cout << "El portal esta sellado. Derrota al jefe primero.\n";
            return;
        }

        int siguienteNivel = nivelActual + 1;
        if (!std::ifstream(Config::mapaPath(siguienteNivel)).good()) {
            std::cout << "Has completado todos los niveles!\n";
            haGanadoFinal = true;
            jugador.setHaGanado(true);
            guardarPartida();
        } else if (cargarNivel(siguienteNivel)) {
            std::cout << "Has avanzado al nivel " << siguienteNivel << "!\n";
            guardarPartida();
        }
    });

    auto manejadorCuracion = [this](GameManager&, int x, int y) {
        char tile = mapa.getTile(x, y);
        int pct = 100;
        auto healing = metadata.healing.find(tile);
        if (healing != metadata.healing.end()) pct = healing->second;
        int amount = jugador.getSaludMaxima() * pct / 100;
        jugador.setSalud(std::min(jugador.getSaludMaxima(), jugador.getSalud() + amount));

        // El tile de curacion no tiene zona propia; restaurar el terreno base del nivel.
        char terrenoBase = '.';
        for (const auto& candidate : metadata.zones) {
            if (candidate.tile != '\0' && candidate.tile != 's') {
                terrenoBase = candidate.tile;
                break;
            }
        }
        mapa.setTile(x, y, terrenoBase);
        CacheManager::guardarMapa(mapa);
    };
    tileRegistry.registrarManejador('H', manejadorCuracion);
    tileRegistry.registrarManejador('h', manejadorCuracion);
    tileRegistry.registrarManejador('G', manejadorCuracion);
}

/**
 * Procesa tiles especiales del mapa mediante el registro TileRegistry.
 */
void GameManager::handleTile(char tile) {
    tileRegistry.ejecutarManejador(tile, *this, jugador.getPosX(), jugador.getPosY());
}

void GameManager::mostrarInventario() {
    InventoryUI invUI(jugador);
    invUI.run();
    // Al salir, la pantalla tiene basura del inventario
    // asi que limpias y re-renderizas el mapa
    Platform::clearScreen();
    renderMapa();
}

/**
 * Inicia un combate contra un enemigo aleatorio.
 * EnemyFactory selecciona una plantilla mediante peso ponderado
 * y crea una instancia de Enemigo lista para batalla().
 */
void GameManager::iniciarCombate() {
    const ZoneMetadata* zone = zonaActual();
    Enemigo enemigo = zone && !zone->enemies.empty()
        ? enemyFactory.crearEnemigo(zone->enemies, zone->statMultiplier)
        : enemyFactory.crearEnemigo(jugador.getNivel());
    auto color = metadata.tierColors.find(enemigo.getTier());
    enemigo.setDisplayColor(color != metadata.tierColors.end() ? color->second : COL_WHITE);
    batalla(jugador, enemigo);
}

/**
 * Inicia un combate contra el jefe del nivel actual.
 * Si no hay jefe definido para el nivel del jugador,
 * muestra un mensaje y cae en un combate aleatorio normal.
 */
void GameManager::iniciarCombateJefe() {
    const ZoneMetadata* zone = zonaJefeActual();
    if (zone && !zone->bossId.empty()) {
        Enemigo jefe = enemyFactory.crearPorId(zone->bossId);
        jefe.aplicarMultiplicadorStats(zone->statMultiplier);
        auto color = metadata.tierColors.find(jefe.getTier());
        jefe.setDisplayColor(color != metadata.tierColors.end() ? color->second : COL_BRED);
        bool esUltimoNivel = !std::ifstream(Config::mapaPath(nivelActual + 1)).good();
        batalla(jugador, jefe, true, esUltimoNivel);
    } else {
        std::cerr << "Error de configuracion: no hay jefe para esta zona.\n";
    }
}

bool GameManager::cargarNivel(int nivel){
    std::string path = Config::mapaPath(nivel);
    if(!mapa.cargar(path)){
        std::cerr << "No se pudo cargar el nivel " << nivel << "\n";
        return false;
    }

encounterMgr.configurarPorNivel(nivel);
    cargarMetadata(nivel);

    nivelActual = nivel;
    jugador.setNivelActual(nivelActual);
    jefeDerrotado = false;

    // El spawn del mapa nuevo se convierte en la posición inicial del jugador.
    for (int y = 0; y < mapa.getAlto(); y++){
        for (int x = 0; x < mapa.getAncho(); x++){
            if(mapa.getTile(x, y) == 'P'){
                jugador.setPos(x, y);
                mapa.setTile(x, y, 's');
                y = mapa.getAlto();
                break;
            }
        }
    }

CacheManager::guardarPartida(mapa, jugador);
    CacheManager::guardarEstado({nivelActual, jefeDerrotado, haGanadoFinal});
    return true;
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
                    Platform::clearScreen();
                    renderMapa();

                    std::cout << "\nWASD para mover, I inventario, Q salir: ";
                    char input = Platform::getKey();
                    int dx = 0, dy = 0;
                    switch (input) {
                    case 'w': case 'W': dy = -1; break;
                    case 's': case 'S': dy = 1; break;
                    case 'a': case 'A': dx = -1; break;
                    case 'd': case 'D': dx = 1; break;
                    case 'i': case 'I': 
                        mostrarInventario(); 
                        continue;
                                case '8': case static_cast<char>(-8):
                                    limiteNivelActivo = !limiteNivelActivo;
                                    jugador.setIgnorarLimiteNivel(!limiteNivelActivo);
                                    continue;
                                case 'q': case 'Q':
                        guardarPartida();
                        return;
                    default: continue;
                    }

                    moverJugador(dx, dy);
                }

                if (jugador.getHaGanado()) { // Falta pantalla de salida
                    std::cout << "\nMuchas gracias por Jugar :)" << std::endl;
                    std::cout << "\n==== PULSA CUALQUIER TECLA PARA SALIR ====\n";
                    char salida;
                    std::cin >> salida; 
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