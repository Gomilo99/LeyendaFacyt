#include <iostream>
#include <algorithm>
#include <string>
#include <limits>
#include <random>
#include <thread>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif
#include "../lib/batalla.hpp"
#include "../lib/DataManager.hpp"

// Limpia el buffer de entrada: descarta hasta encontrar \n
void limpiarBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Limpia la terminal usando codigo ANSI
void limpiarPantalla() {
    std::cout << "\033[2J\033[1;1H";
}

// ==================== SCREEN BUFFER ====================

// Inicializa el buffer: firstFrame=true fuerza redibujado completo en el primer render()
ScreenBuffer::ScreenBuffer() : firstFrame(true) {
    clear();
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
            prev[y][x] = '\0';
}

// Copia el estado actual (grid + attrs) a los buffers anteriores (prev + prevAttrs)
void ScreenBuffer::syncPrev() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            prev[y][x] = grid[y][x];
            prevAttrs[y][x] = attrs[y][x];
        }
    }
}

// Llena todo el buffer con espacios y resetea colores
void ScreenBuffer::clear() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            grid[y][x] = ' ';
            attrs[y][x] = 0;
        }
    }
}

// Escribe un caracter en (x,y) con color opcional. Ignora coordenadas fuera de rango.
void ScreenBuffer::setChar(int x, int y, char c, int color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        grid[y][x] = c;
        if (color) attrs[y][x] = color;
    }
}

// Cambia solo el color de una celda sin modificar el caracter
void ScreenBuffer::setAttr(int x, int y, int color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
        attrs[y][x] = color;
}

// Dibuja texto horizontal desde (x,y)
void ScreenBuffer::drawString(int x, int y, const std::string& str, int color) {
    for (size_t i = 0; i < str.size(); i++)
        setChar(x + (int)i, y, str[i], color);
}

// Dibuja linea horizontal de longitud w con el caracter c
void ScreenBuffer::drawHLine(int x, int y, int w, char c, int color) {
    for (int i = 0; i < w; i++) setChar(x + i, y, c, color);
}

// Dibuja linea vertical de altura h con el caracter c
void ScreenBuffer::drawVLine(int x, int y, int h, char c, int color) {
    for (int i = 0; i < h; i++) setChar(x, y + i, c, color);
}

// Dibuja un recuadro de w x h usando '=' para bordes horizontales y '|' para verticales
void ScreenBuffer::drawBox(int x, int y, int w, int h, int color) {
    drawHLine(x, y, w, '=', color);
    drawHLine(x, y + h - 1, w, '=', color);
    drawVLine(x, y, h, '|', color);
    drawVLine(x + w - 1, y, h, '|', color);
    setChar(x, y, '+', color);
    setChar(x + w - 1, y, '+', color);
    setChar(x, y + h - 1, '+', color);
    setChar(x + w - 1, y + h - 1, '+', color);
}

// Dibuja una barra de progreso: '#' para el porcentaje completado, '.' para el resto
void ScreenBuffer::drawBar(int x, int y, int w, int current, int max, int color) {
    int fill = (max > 0) ? (current * w / max) : 0;
    fill = std::max(0, std::min(w, fill));
    int fillColor = color ? color : COL_GREEN;
    int emptyColor = COL_DEFAULT;
    drawHLine(x, y, fill, '#', fillColor);
    drawHLine(x + fill, y, w - fill, '.', emptyColor);
}

// Vuelca el buffer a la terminal con redibujado diferencial:
// solo las lineas que cambiaron se reescriben, usando posicionamiento ANSI.
void ScreenBuffer::render() {
    // Primer frame: limpiar pantalla completa e invalidar prev
    if (firstFrame) {
        std::cout << "\033[2J";
        firstFrame = false;
        for (int y = 0; y < SCREEN_HEIGHT; y++)
            for (int x = 0; x < SCREEN_WIDTH; x++)
                prev[y][x] = '\0';
    }

    // Comparar cada linea contra su version anterior
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        bool lineSame = true;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (grid[y][x] != prev[y][x] || attrs[y][x] != prevAttrs[y][x]) {
                lineSame = false;
                break;
            }
        }
        if (lineSame) continue; // saltar lineas sin cambios

        // Posicionar cursor al inicio de la linea (1-indexed)
        std::cout << "\033[" << (y + 1) << ";1H";
        int currentColor = 0;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int c = attrs[y][x];
            // Emitir codigo ANSI solo si el color cambio
            if (c != currentColor) {
                if (c) std::cout << "\033[" << c << "m";
                else   std::cout << "\033[0m";
                currentColor = c;
            }
            std::cout << grid[y][x];
        }
        if (currentColor) std::cout << "\033[0m";
    }

    // Posicionar cursor al final del area de juego para evitar parpadeo
    std::cout << "\033[" << SCREEN_HEIGHT << ";1H";
    std::cout.flush();
    syncPrev();
}

// Invalida el buffer anterior para forzar un redibujado completo en el proximo render()
void ScreenBuffer::forceRedraw() {
    firstFrame = true;
}

// Oculta el cursor de la terminal usando secuencia ANSI DECTCEM
void ScreenBuffer::hideCursor() {
    std::cout << "\033[?25l";
    std::cout.flush();
}

// Muestra el cursor de la terminal
void ScreenBuffer::showCursor() {
    std::cout << "\033[?25h";
    std::cout.flush();
}

// Consulta el ancho de la terminal via Win32 Console API. Fallback a 80.
int ScreenBuffer::getTerminalWidth() {
    #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    #endif
    return 80;
}

// Consulta el alto de la terminal via Win32 Console API. Fallback a 24.
int ScreenBuffer::getTerminalHeight() {
    #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    #endif
    return 24;
}

// ==================== RENDERER ====================

Renderer::Renderer(ScreenBuffer& buffer)
    : buf(buffer), selOpt(0), enemyHP(0), enemyMaxHP(1),
      playerHP(0), playerMaxHP(1), playerMP(0), playerMaxMP(1) {}

// Almacena la informacion del enemigo para el proximo frame
void Renderer::setEnemyInfo(const std::string& name, int hp, int maxHp, const std::string art[6]) {
    enemyName = name;
    enemyHP = hp;
    enemyMaxHP = std::max(maxHp, 1);
    for (int i = 0; i < 6; i++) enemyArt[i] = art[i];
}

// Almacena la informacion del jugador para el proximo frame
void Renderer::setPlayerInfo(const std::string& name, int hp, int maxHp, int mp, int maxMp) {
    playerName = name;
    playerHP = hp;
    playerMaxHP = std::max(maxHp, 1);
    playerMP = mp;
    playerMaxMP = std::max(maxMp, 1);
}

// Dibuja el encabezado del frame de combate
void Renderer::drawBackground() {
    buf.drawString(0, 0, "=== LEYENDA DEL CAMPUS - COMBATE ===", COL_BYELLOW);
    buf.drawHLine(0, 1, SCREEN_WIDTH, '-', COL_CYAN);
}

// Dibuja el recuadro con nombre del enemigo (centrado) y 6 lineas de arte ASCII
void Renderer::drawEnemy() {
    int cx = SCREEN_WIDTH / 2;
    int nameLen = (int)enemyName.size();
    int boxW = std::max(nameLen + 4, 18);
    int boxX = cx - boxW / 2;

    buf.drawBox(boxX, 2, boxW, 2, COL_CYAN);
    buf.drawString(cx - nameLen / 2, 3, enemyName, COL_BRED);

    for (int i = 0; i < 6; i++) {
        int artX = cx - (int)enemyArt[i].size() / 2;
        buf.drawString(artX, 6 + i, enemyArt[i], COL_WHITE);
    }
}

// Dibuja barra de vida del enemigo con color variable segun % de HP
void Renderer::drawEnemyHealthBar() {
    int cx = SCREEN_WIDTH / 2;
    int barW = 30;
    int barX = cx - barW / 2;
    std::string hpText = "HP: " + std::to_string(enemyHP) + "/" + std::to_string(enemyMaxHP);
    buf.drawString(cx - (int)hpText.size() / 2, 4, hpText, COL_BWHITE);

    int pct = (enemyMaxHP > 0) ? (enemyHP * 100 / enemyMaxHP) : 0;
    int barColor = (pct > 50) ? COL_GREEN : (pct > 25) ? COL_YELLOW : COL_RED;
    buf.drawBar(barX, 5, barW, enemyHP, enemyMaxHP, barColor);
}

// Dibuja el menu de acciones del jugador (Atacar, Magia, Inventario, Huir)
// con la opcion seleccionada resaltada en amarillo brillante
void Renderer::drawCombatMenu() {
    int menuX = 3;
    int menuY = 12;
    int menuW = 22;
    int menuH = 7;

    buf.drawBox(menuX, menuY, menuW, menuH, COL_CYAN);

    const char* options[] = { "Atacar", "Magia", "Inventario", "Huir" };
    for (int i = 0; i < 4; i++) {
        int optY = menuY + 1 + i;
        int selColor = (i == selOpt) ? COL_BYELLOW : COL_DEFAULT;
        std::string line = (i == selOpt) ? " > " + std::string(options[i])
                                         : "   " + std::string(options[i]);
        buf.drawString(menuX + 2, optY, line, selColor);
    }

    buf.drawString(menuX + 2, menuY + 5, "[W/S] Navegar [SPACE] OK", COL_CYAN);
}

// Dibuja el panel del jugador con nombre, barra de HP (verde/amarillo/rojo) y MP (azul)
void Renderer::drawPlayerInfo() {
    int infoX = SCREEN_WIDTH - 28;
    int infoY = 12;
    int infoW = 26;
    int infoH = 7;

    buf.drawBox(infoX, infoY, infoW, infoH, COL_CYAN);
    buf.drawString(infoX + 2, infoY + 1, playerName, COL_BWHITE);

    buf.drawString(infoX + 2, infoY + 2, "HP", COL_GREEN);
    int hpPct = (playerMaxHP > 0) ? (playerHP * 100 / playerMaxHP) : 0;
    int hpColor = (hpPct > 50) ? COL_GREEN : (hpPct > 25) ? COL_YELLOW : COL_RED;
    buf.drawBar(infoX + 5, infoY + 2, 16, playerHP, playerMaxHP, hpColor);

    buf.drawString(infoX + 2, infoY + 3, "MP", COL_BLUE);
    buf.drawBar(infoX + 5, infoY + 3, 16, playerMP, playerMaxMP, COL_BLUE);

    std::string hpNum = std::to_string(playerHP) + "/" + std::to_string(playerMaxHP);
    buf.drawString(infoX + infoW - 1 - (int)hpNum.size(), infoY + 4, hpNum, hpColor);
    std::string mpNum = std::to_string(playerMP) + "/" + std::to_string(playerMaxMP);
    buf.drawString(infoX + infoW - 1 - (int)mpNum.size(), infoY + 5, mpNum, COL_BLUE);
}

// Dibuja el mensaje de log centrado en la ultima linea del buffer
void Renderer::drawLog() {
    if (!logMsg.empty()) {
        int cx = SCREEN_WIDTH / 2;
        buf.drawString(cx - (int)logMsg.size() / 2, SCREEN_HEIGHT - 1, logMsg, COL_BYELLOW);
    }
}

// Renderiza el frame completo: limpia buffer, dibuja todas las secciones, presenta en terminal
void Renderer::renderAll() {
    buf.clear();
    drawBackground();
    drawEnemy();
    drawEnemyHealthBar();
    drawCombatMenu();
    drawPlayerInfo();
    drawLog();
    buf.render();
}

// ==================== INPUT HANDLER ====================

InputHandler::InputHandler() : selectedOption(0) {}

// Mueve la seleccion hacia arriba (circular, 4 opciones)
void InputHandler::moveUp() {
    selectedOption = (selectedOption - 1 + 4) % 4;
}

// Mueve la seleccion hacia abajo (circular, 4 opciones)
void InputHandler::moveDown() {
    selectedOption = (selectedOption + 1) % 4;
}

// ==================== BATTLE SYSTEM ====================

BattleSystem::BattleSystem(Jugador& p, Enemigo& e)
    : currentState(BattleState::PLAYER_TURN), currentEnemy(&e), player(&p),
      screenBuffer(), renderer(screenBuffer), inputHandler(),
      battleOver(false), victory(false), fled(false) {
    const std::string* art = e.getAsciiArt();
    for (int i = 0; i < 6; i++) enemyArt[i] = art[i];
}

// Redirige cout a un stringstream interno para silenciar salida durante acciones de combate
void BattleSystem::suppressCout() {
    oldCoutBuf = std::cout.rdbuf();
    std::cout.rdbuf(coutSink.rdbuf());
}

// Restaura cout a su buffer original y descarta lo acumulado
void BattleSystem::restoreCout() {
    std::cout.rdbuf(oldCoutBuf);
    coutSink.str("");
    coutSink.clear();
}

void BattleSystem::setLog(const std::string& msg) {
    logMessage = msg;
    renderer.setLogMessage(logMessage);
}

// Procesa entrada del jugador en estado PLAYER_TURN:
// W/S navegan el menu, SPACE confirma la seleccion actual
void BattleSystem::processInput() {
    if (currentState != BattleState::PLAYER_TURN) return;

    char key;
    if (std::cin.get(key)) {
        if (key == 'w' || key == 'W') {
            inputHandler.moveUp();
            renderer.setSelectedOption(inputHandler.getSelectedOption());
        } else if (key == 's' || key == 'S') {
            inputHandler.moveDown();
            renderer.setSelectedOption(inputHandler.getSelectedOption());
        } else if (key == ' ') {
            currentState = BattleState::PLAYER_ACTION;
        }
    }
}

// Ejecuta la accion correspondiente segun la opcion seleccionada (0-3)
void BattleSystem::doPlayerAction() {
    int opt = inputHandler.getSelectedOption();
    switch (opt) {
        case 0: // Atacar
            setLog(player->getNombre() + " ataca a " + currentEnemy->getNombre() + "!");
            render();
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            suppressCout();
            player->atacar(currentEnemy);
            restoreCout();
            if (!currentEnemy->estaVivo()) {
                setLog("Has derrotado a " + currentEnemy->getNombre() + "!");
                render();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                victory = true;
                battleOver = true;
                currentState = BattleState::VICTORY;
                return;
            }
            currentState = BattleState::ENEMY_TURN;
            break;

        case 1: // Magia: requiere 10 MP minimo
            if (player->getMana() < 10) {
                setLog("No tienes suficiente mana! (10 MP)");
                render();
                std::this_thread::sleep_for(std::chrono::milliseconds(800));
                currentState = BattleState::PLAYER_TURN;
                return;
            }
            setLog(player->getNombre() + " lanza un hechizo!");
            render();
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            suppressCout();
            player->usarMagia(currentEnemy);
            restoreCout();
            if (!currentEnemy->estaVivo()) {
                setLog("Has derrotado a " + currentEnemy->getNombre() + "!");
                render();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                victory = true;
                battleOver = true;
                currentState = BattleState::VICTORY;
                return;
            }
            currentState = BattleState::ENEMY_TURN;
            break;

        case 2: // Inventario: muestra estado e inventario, permite usar objetos por nombre
            limpiarPantalla();
            player->mostrarEstado();
            std::cout << "\n--- Inventario ---\n";
            {
                auto& inv = player->getInventario();
                if (inv.empty()) {
                    std::cout << "(vacio)\n";
                } else {
                    for (const auto& par : inv) {
                        std::cout << "- " << par.first << " x" << par.second << "\n";
                    }
                }
            }
            std::cout << "\nPociones: " << player->getPociones() << "\n";
            std::cout << "\nEscribe el nombre del objeto a usar, o Enter para volver: ";
            limpiarBuffer();
            {
                std::string nombreObj;
                std::getline(std::cin, nombreObj);
                if (!nombreObj.empty()) {
                    auto it = player->getObjetosInventario().find(nombreObj);
                    if (it != player->getObjetosInventario().end()) {
                        player->usarPocion(it->second.get());
                        player->eliminarObjeto(nombreObj);
                    } else {
                        std::cout << "No tienes \"" << nombreObj << "\" en tu inventario.\n";
                    }
                    std::cout << "Presiona Enter para continuar...";
                    limpiarBuffer();
                    std::cin.get();
                }
            }
            screenBuffer.forceRedraw();
            currentState = BattleState::PLAYER_TURN;
            break;

        case 3: // Huir: 50% de probabilidad de exito
        {
            std::uniform_int_distribution<int> dist(0, 99);
            int chance = dist(DataManager::rng());
            if (chance < 50) {
                setLog("Has huido exitosamente!");
                render();
                std::this_thread::sleep_for(std::chrono::milliseconds(800));
                fled = true;
                battleOver = true;
                currentState = BattleState::FLEE;
            } else {
                setLog("No lograste huir!");
                render();
                std::this_thread::sleep_for(std::chrono::milliseconds(800));
                currentState = BattleState::ENEMY_TURN;
            }
            break;
        }
    }
}

// Turno del enemigo: muestra mensaje de ataque, pausa animada, ejecuta ataque
void BattleSystem::doEnemyTurn() {
    setLog(currentEnemy->getNombre() + " te ataca!");
    render();
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    suppressCout();
    currentEnemy->atacar(player);
    restoreCout();

    if (!player->estaVivo()) {
        setLog("Has sido derrotado!");
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        battleOver = true;
        currentState = BattleState::DEFEAT;
    } else {
        currentState = BattleState::PLAYER_TURN;
    }
}

// Compone el frame de combate actual: pasa datos de enemigo/jugador al Renderer y lo dibuja
void BattleSystem::render() {
    renderer.setEnemyInfo(
        currentEnemy->getNombre(),
        currentEnemy->getSalud(),
        currentEnemy->getSaludMaxima(),
        enemyArt
    );
    renderer.setPlayerInfo(
        player->getNombre(),
        player->getSalud(),
        player->getSaludMaxima(),
        player->getMana(),
        player->getManaMaxima()
    );
    renderer.setLogMessage(logMessage);
    renderer.renderAll();
}

// Bucle principal del combate:
// 1. Oculta el cursor
// 2. Ciclo: PLAYER_TURN (input) → doPlayerAction → ENEMY_TURN → loop
// 3. Muestra el cursor al terminar
void BattleSystem::run() {
    ScreenBuffer::hideCursor();
    renderer.setSelectedOption(0);

    while (!battleOver) {
        // Reiniciar a turno del jugador
        currentState = BattleState::PLAYER_TURN;
        inputHandler.setSelectedOption(0);
        renderer.setSelectedOption(0);
        logMessage = "Selecciona una accion (W/S, SPACE)";

        // Bucle de entrada: renderiza y procesa teclas hasta que confirme
        while (currentState == BattleState::PLAYER_TURN && !battleOver) {
            render();
            processInput();
        }

        if (battleOver) break;

        doPlayerAction();

        if (battleOver) break;

        // Turno del enemigo
        currentState = BattleState::ENEMY_TURN;
        doEnemyTurn();
    }

    render();
    ScreenBuffer::showCursor();
}

// Punto de entrada al combate desde el mapa:
// 1. Muestra intro (jefe final o enemigo normal)
// 2. Instancia y ejecuta BattleSystem
// 3. Post-batalla: maneja huida, derrota, victoria, experiencia y loot
void batalla(Jugador& jugador, Enemigo& enemigo) {
    limpiarPantalla();

    bool jefefinal = (enemigo.getNivel() >= 4);

    if (jefefinal) {
        std::cout << "Estas en una oscura cueva, sientes una presencia extrana...\n";
        std::cout << "Ha aparecido el jefe final!!!\nHa aparecido " << enemigo.getNombre() << std::endl;
    } else {
        std::cout << "Un " << enemigo.getNombre() << " ha aparecido!\n";
    }
    std::cout << "Presiona Enter para comenzar la batalla...";
    limpiarBuffer();
    std::cin.get();

    BattleSystem system(jugador, enemigo);
    system.run();

    if (system.hasFled()) {
        std::cout << "\nHas escapado del combate.\nPresiona Enter para continuar...";
        limpiarBuffer();
        std::cin.get();
        return;
    }

    if (!jugador.estaVivo()) {
        std::cout << "\nHas sido derrotado por " << enemigo.getNombre() << "!\n";
        DataManager::guardarHeroe(jugador);
        std::cout << "Presiona Enter para continuar...";
        limpiarBuffer();
        std::cin.get();
        return;
    }

    // Victoria
    limpiarPantalla();
    std::cout << "\n\nHAS DERROTADO A '" << enemigo.getNombre() << "' !\n";
    if (jefefinal) {
        std::cout << "Felicidades, has derrotado al jefe final!\n";
        std::cout << "...\n";
        std::cout << "Has ganado el juego!\n";
        jugador.setHaGanado(true);
    }

    // Otorgar experiencia
    int exp = jugador.getNivel() * 50;
    jugador.obtenerExperiencia(exp);

    // Calcular loot segun probabilidades del enemigo (recorre el vector botin)
    std::uniform_int_distribution<int> distLoot(0, 99);
    int chance = distLoot(DataManager::rng());
    std::shared_ptr<Objeto> lootGanado = nullptr;

    const auto& botin = enemigo.getBotin();
    int acumulado = 0;
    for (const auto& drop : botin) {
        acumulado += drop.probabilidad;
        if (chance < acumulado) {
            lootGanado = drop.objeto;
            break;
        }
    }

    if (lootGanado) {
        std::cout << "Has obtenido: " << lootGanado->getNombre() << "\n";
        jugador.agregarObjeto(lootGanado);
    }

    DataManager::guardarHeroe(jugador);
    std::cout << "Presiona Enter para continuar...";
    limpiarBuffer();
    std::cin.get();
}
