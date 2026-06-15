#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <limits>
#include <map>
#include <memory>
#include <vector>
#include <random>
#include <cstdlib>
#include <thread>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif
#include "../lib/batalla.hpp"

std::mt19937& rng() {
    static std::mt19937 engine(std::random_device{}());
    return engine;
}

void limpiarBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void limpiarPantalla() {
    std::cout << "\033[2J\033[1;1H";
}

// ==================== SCREEN BUFFER ====================

ScreenBuffer::ScreenBuffer() : firstFrame(true) {
    clear();
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
            prev[y][x] = '\0';
}

void ScreenBuffer::syncPrev() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            prev[y][x] = grid[y][x];
            prevAttrs[y][x] = attrs[y][x];
        }
    }
}

void ScreenBuffer::clear() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            grid[y][x] = ' ';
            attrs[y][x] = 0;
        }
    }
}

void ScreenBuffer::setChar(int x, int y, char c, int color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        grid[y][x] = c;
        if (color) attrs[y][x] = color;
    }
}

void ScreenBuffer::setAttr(int x, int y, int color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
        attrs[y][x] = color;
}

void ScreenBuffer::drawString(int x, int y, const std::string& str, int color) {
    for (size_t i = 0; i < str.size(); i++)
        setChar(x + (int)i, y, str[i], color);
}

void ScreenBuffer::drawHLine(int x, int y, int w, char c, int color) {
    for (int i = 0; i < w; i++) setChar(x + i, y, c, color);
}

void ScreenBuffer::drawVLine(int x, int y, int h, char c, int color) {
    for (int i = 0; i < h; i++) setChar(x, y + i, c, color);
}

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

void ScreenBuffer::drawBar(int x, int y, int w, int current, int max, int color) {
    int fill = (max > 0) ? (current * w / max) : 0;
    fill = std::max(0, std::min(w, fill));
    int fillColor = color ? color : COL_GREEN;
    int emptyColor = COL_DEFAULT;
    drawHLine(x, y, fill, '#', fillColor);
    drawHLine(x + fill, y, w - fill, '.', emptyColor);
}

void ScreenBuffer::render() {
    if (firstFrame) {
        std::cout << "\033[2J";
        firstFrame = false;
        for (int y = 0; y < SCREEN_HEIGHT; y++)
            for (int x = 0; x < SCREEN_WIDTH; x++)
                prev[y][x] = '\0';
    }

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        bool lineSame = true;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (grid[y][x] != prev[y][x] || attrs[y][x] != prevAttrs[y][x]) {
                lineSame = false;
                break;
            }
        }
        if (lineSame) continue;

        std::cout << "\033[" << (y + 1) << ";1H";
        int currentColor = 0;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int c = attrs[y][x];
            if (c != currentColor) {
                if (c) std::cout << "\033[" << c << "m";
                else   std::cout << "\033[0m";
                currentColor = c;
            }
            std::cout << grid[y][x];
        }
        if (currentColor) std::cout << "\033[0m";
    }

    std::cout << "\033[" << SCREEN_HEIGHT << ";1H";
    std::cout.flush();
    syncPrev();
}

void ScreenBuffer::forceRedraw() {
    firstFrame = true;
}

void ScreenBuffer::hideCursor() {
    std::cout << "\033[?25l";
    std::cout.flush();
}

void ScreenBuffer::showCursor() {
    std::cout << "\033[?25h";
    std::cout.flush();
}

int ScreenBuffer::getTerminalWidth() {
    #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    #endif
    return 80;
}

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

void Renderer::setEnemyInfo(const std::string& name, int hp, int maxHp, const std::string art[6]) {
    enemyName = name;
    enemyHP = hp;
    enemyMaxHP = std::max(maxHp, 1);
    for (int i = 0; i < 6; i++) enemyArt[i] = art[i];
}

void Renderer::setPlayerInfo(const std::string& name, int hp, int maxHp, int mp, int maxMp) {
    playerName = name;
    playerHP = hp;
    playerMaxHP = std::max(maxHp, 1);
    playerMP = mp;
    playerMaxMP = std::max(maxMp, 1);
}

void Renderer::drawBackground() {
    buf.drawString(0, 0, "=== LEYENDA DEL CAMPUS - COMBATE ===", COL_BYELLOW);
    buf.drawHLine(0, 1, SCREEN_WIDTH, '-', COL_CYAN);
}

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

void Renderer::drawLog() {
    if (!logMsg.empty()) {
        int cx = SCREEN_WIDTH / 2;
        buf.drawString(cx - (int)logMsg.size() / 2, SCREEN_HEIGHT - 1, logMsg, COL_BYELLOW);
    }
}

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

void InputHandler::moveUp() {
    selectedOption = (selectedOption - 1 + 4) % 4;
}

void InputHandler::moveDown() {
    selectedOption = (selectedOption + 1) % 4;
}

// ==================== BATTLE SYSTEM ====================

BattleSystem::BattleSystem(Jugador& p, Enemigo& e)
    : currentState(BattleState::PLAYER_TURN), currentEnemy(&e), player(&p),
      screenBuffer(), renderer(screenBuffer), inputHandler(),
      battleOver(false), victory(false), fled(false) {
    for (int i = 0; i < 6; i++) enemyArt[i] = "";
    generateEnemyArt();
}

void BattleSystem::generateEnemyArt() {
    std::string name = currentEnemy->getNombre();
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower.find("dragon") != std::string::npos || lower.find("admin") != std::string::npos) {
        enemyArt[0] = "     /\\/\\/\\/\\/\\/\\/\\/\\";
        enemyArt[1] = "    /     BOSS        \\";
        enemyArt[2] = "   /  /\\          /\\  \\";
        enemyArt[3] = "  /  /  \\        /  \\  \\";
        enemyArt[4] = " /  /    \\      /    \\  \\";
        enemyArt[5] = "/__/______\\____/______\\__\\";
    } else if (lower.find("golem") != std::string::npos || lower.find("ogro") != std::string::npos) {
        enemyArt[0] = "      ___________";
        enemyArt[1] = "     /           \\";
        enemyArt[2] = "    |  O       O  |";
        enemyArt[3] = "    |      _      |";
        enemyArt[4] = "    |     |_|     |";
        enemyArt[5] = "    |___/    \\___|";
    } else if (lower.find("fantasma") != std::string::npos || lower.find("espectro") != std::string::npos) {
        enemyArt[0] = "       .-.";
        enemyArt[1] = "      (o o)";
        enemyArt[2] = "      | O |";
        enemyArt[3] = "     /|___|\\";
        enemyArt[4] = "    / /   \\ \\";
        enemyArt[5] = "   / /     \\ \\";
    } else if (lower.find("esqueleto") != std::string::npos) {
        enemyArt[0] = "      .-.";
        enemyArt[1] = "     ( . )";
        enemyArt[2] = "      /|\\";
        enemyArt[3] = "     / | \\";
        enemyArt[4] = "       |";
        enemyArt[5] = "      / \\";
    } else if (lower.find("cajero") != std::string::npos) {
        enemyArt[0] = "  +-----------+";
        enemyArt[1] = "  | [__] [__] |";
        enemyArt[2] = "  |  _    _   |";
        enemyArt[3] = "  | |_|  |_|  |";
        enemyArt[4] = "  |    ___    |";
        enemyArt[5] = "  +-----------+";
    } else if (lower.find("gargola") != std::string::npos || lower.find("caballero") != std::string::npos) {
        enemyArt[0] = "     /\\";
        enemyArt[1] = "    /  \\";
        enemyArt[2] = "   | ._.|";
        enemyArt[3] = "   |_/ \\_|";
        enemyArt[4] = "    |   |";
        enemyArt[5] = "   /     \\";
    } else if (lower.find("ciclope") != std::string::npos) {
        enemyArt[0] = "     ______";
        enemyArt[1] = "    | O   |";
        enemyArt[2] = "    |  _  |";
        enemyArt[3] = "    | | | |";
        enemyArt[4] = "    |_| |_|";
        enemyArt[5] = "     /   \\";
    } else if (lower.find("slime") != std::string::npos) {
        enemyArt[0] = "     .-.";
        enemyArt[1] = "    (o o)";
        enemyArt[2] = "    | ~ |";
        enemyArt[3] = "    \\___/";
        enemyArt[4] = "   /     \\";
        enemyArt[5] = "  /       \\";
    } else if (lower.find("goblin") != std::string::npos || lower.find("duende") != std::string::npos) {
        enemyArt[0] = "     /\\";
        enemyArt[1] = "    /  \\";
        enemyArt[2] = "   | <> |";
        enemyArt[3] = "   | <> |";
        enemyArt[4] = "   /    \\";
        enemyArt[5] = "  /______\\";
    } else if (lower.find("orco") != std::string::npos) {
        enemyArt[0] = "    /^^^\\";
        enemyArt[1] = "   | . . |";
        enemyArt[2] = "   |  _  |";
        enemyArt[3] = "   | / \\ |";
        enemyArt[4] = "   |/   \\|";
        enemyArt[5] = "  _/     \\_";
    } else if (lower.find("zombie") != std::string::npos || lower.find("bruja") != std::string::npos) {
        enemyArt[0] = "     ___";
        enemyArt[1] = "    / . \\";
        enemyArt[2] = "   |  _  |";
        enemyArt[3] = "   | / \\ |";
        enemyArt[4] = "   |/   \\|";
        enemyArt[5] = "  _/     \\_";
    } else {
        enemyArt[0] = "     /\\";
        enemyArt[1] = "    /  \\";
        enemyArt[2] = "   | {} |";
        enemyArt[3] = "   | {} |";
        enemyArt[4] = "   /    \\";
        enemyArt[5] = "  /______\\";
    }
}

void BattleSystem::suppressCout() {
    oldCoutBuf = std::cout.rdbuf();
    std::cout.rdbuf(coutSink.rdbuf());
}

void BattleSystem::restoreCout() {
    std::cout.rdbuf(oldCoutBuf);
    coutSink.str("");
    coutSink.clear();
}

void BattleSystem::setLog(const std::string& msg) {
    logMessage = msg;
    renderer.setLogMessage(logMessage);
}

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
            inputHandler.getSelectedOption(); // just to keep interface clean
        }
    }
}

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

        case 1: // Magia
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

        case 2: // Inventario
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

        case 3: // Huir
        {
            std::uniform_int_distribution<int> dist(0, 99);
            int chance = dist(rng());
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

void BattleSystem::run() {
    ScreenBuffer::hideCursor();
    renderer.setSelectedOption(0);

    while (!battleOver) {
        currentState = BattleState::PLAYER_TURN;
        inputHandler.setSelectedOption(0);
        renderer.setSelectedOption(0);
        logMessage = "Selecciona una accion (W/S, SPACE)";

        while (currentState == BattleState::PLAYER_TURN && !battleOver) {
            render();
            processInput();
        }

        if (battleOver) break;

        doPlayerAction();

        if (battleOver) break;

        currentState = BattleState::ENEMY_TURN;
        doEnemyTurn();
    }

    render();
    ScreenBuffer::showCursor();
}

// ==================== DATA LOADING ====================

std::map<std::string, std::shared_ptr<Objeto>> cargarObjetosDesdeJSON(const std::string& archivo) {
    std::map<std::string, std::shared_ptr<Objeto>> objetos;
    std::ifstream file(archivo);
    if (!file.is_open()) return objetos;
    json j;
    file >> j;

    if (j.contains("arma")) {
        for (const auto& item : j["arma"]) {
            std::string nombre = item["nombre"];
            std::string descripcion = item["descripcion"];
            int dano = item["dano"];
            objetos[nombre] = std::make_shared<Arma>(nombre, descripcion, dano);
        }
    }
    if (j.contains("pocion")) {
        for (const auto& item : j["pocion"]) {
            std::string nombre = item["nombre"];
            std::string descripcion = item["descripcion"];
            int curacion = item["curacion"];
            objetos[nombre] = std::make_shared<Pocion>(nombre, descripcion, curacion);
        }
    }
    if (j.contains("clave")) {
        for (const auto& item : j["clave"]) {
            std::string nombre = item["nombre"];
            std::string descripcion = item["descripcion"];
            objetos[nombre] = std::make_shared<ObjClave>(nombre, descripcion);
        }
    }
    file.close();
    return objetos;
}

std::map<std::string, std::shared_ptr<Enemigo>> cargarEnemigosDesdeJSON(
    const std::string& archivo,
    const std::map<std::string, std::shared_ptr<Objeto>>& objetosDisponibles)
{
    std::map<std::string, std::shared_ptr<Enemigo>> enemigos;
    std::ifstream file(archivo);
    if (!file.is_open()) return enemigos;
    json j;
    file >> j;

    for (auto it = j.begin(); it != j.end(); ++it) {
        int nivel = std::stoi(it.key());
        for (const auto& item : it.value()) {
            std::string nombre = item["nombre"];
            int salud = item["salud"];
            int ataque = item["ataque"];
            int defensa = item["defensa"];
            std::string nombreLoot1 = item["loot1"];
            std::string nombreLoot2 = item["loot2"];
            int prob1 = item.contains("prob1") ? (int)item["prob1"] : 70;
            int prob2 = item.contains("prob2") ? (int)item["prob2"] : 30;

            auto it1 = objetosDisponibles.find(nombreLoot1);
            auto it2 = objetosDisponibles.find(nombreLoot2);
            if (it1 == objetosDisponibles.end() || it2 == objetosDisponibles.end()) {
                throw std::runtime_error("Objeto no encontrado: " + nombreLoot1 + " o " + nombreLoot2);
            }

            Drop drop1(it1->second, prob1);
            Drop drop2(it2->second, prob2);

            auto enemigo = std::make_shared<Enemigo>(nombre, salud, ataque, defensa, nivel, drop1, drop2);
            enemigos[nombre] = enemigo;
        }
    }
    file.close();
    return enemigos;
}

Jugador cargarHeroe(const std::string& archivo) {
    std::ifstream file(archivo);
    json j;
    file >> j;
    std::string nombre = j["nombre"];
    int salud = j["salud"];
    int ataque = j["ataque"];
    int defensa = j["defensa"];
    int nivel = j["nivel"];
    int pociones = j.contains("pociones") ? (int)j["pociones"] : 3;
    Jugador jugador(nombre, salud, ataque, defensa, nivel, pociones);
    file.close();
    return jugador;
}

void guardarHeroe(const Jugador& jugador, const std::string& archivo) {
    json j;
    j["nombre"] = jugador.getNombre();
    j["salud"] = jugador.getSalud();
    j["ataque"] = jugador.getAtaque();
    j["defensa"] = jugador.getDefensa();
    j["nivel"] = jugador.getNivel();
    j["pociones"] = jugador.getPociones();
    j["mana"] = jugador.getMana();
    std::ofstream file(archivo);
    file << j.dump(4);
    file.close();
}

std::shared_ptr<Enemigo> generarEnemigoPorNivel(
    const std::map<std::string, std::shared_ptr<Enemigo>>& enemigos, int nivelMaxPermitido)
{
    std::vector<std::shared_ptr<Enemigo>> candidatos;
    for (const auto& par : enemigos) {
        if (par.second->getNivel() == nivelMaxPermitido)
            candidatos.push_back(par.second);
    }
    if (candidatos.empty())
        throw std::runtime_error("No hay enemigos disponibles para el nivel solicitado");
    std::uniform_int_distribution<int> dist(0, candidatos.size() - 1);
    return candidatos[dist(rng())];
}

void batalla(Jugador& jugador, const std::map<std::string, std::shared_ptr<Enemigo>>& enemigos) {
    limpiarPantalla();

    auto enemigoSelec = generarEnemigoPorNivel(enemigos, jugador.getNivel());
    Enemigo enemigo(*enemigoSelec);
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
        guardarHeroe(jugador, dataPath("heroe.json"));
        std::cout << "Presiona Enter para continuar...";
        limpiarBuffer();
        std::cin.get();
        return;
    }

    limpiarPantalla();
    std::cout << "\n\nHAS DERROTADO A '" << enemigo.getNombre() << "' !\n";
    if (jefefinal) {
        std::cout << "Felicidades, has derrotado al jefe final!\n";
        std::cout << "...\n";
        std::cout << "Has ganado el juego!\n";
        jugador.setHaGanado(true);
    }

    int exp = jugador.getNivel() * 50;
    jugador.obtenerExperiencia(exp);

    std::uniform_int_distribution<int> distLoot(0, 99);
    int chance = distLoot(rng());
    std::shared_ptr<Objeto> lootGanado = nullptr;

    Drop loot1 = enemigo.getLoot1();
    Drop loot2 = enemigo.getLoot2();
    if (chance < loot1.probabilidad)
        lootGanado = loot1.objeto;
    else if (chance < loot1.probabilidad + loot2.probabilidad)
        lootGanado = loot2.objeto;

    if (lootGanado) {
        std::cout << "Has obtenido: " << lootGanado->getNombre() << "\n";
        jugador.agregarObjeto(lootGanado);
    }

    guardarHeroe(jugador, dataPath("heroe.json"));
    std::cout << "Presiona Enter para continuar...";
    limpiarBuffer();
    std::cin.get();
}
