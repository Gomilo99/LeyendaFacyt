#ifndef BATALLA_HPP
#define BATALLA_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "json.hpp"
#include "objeto.hpp"
#include "enemigo.hpp"
#include "jugador.hpp"

using json = nlohmann::json;

const int SCREEN_WIDTH = 56;
const int SCREEN_HEIGHT = 22;

enum class BattleState {
    PLAYER_TURN,
    PLAYER_ACTION,
    ENEMY_TURN,
    ANIMATION,
    VICTORY,
    DEFEAT,
    FLEE
};

class ScreenBuffer {
    char grid[SCREEN_HEIGHT][SCREEN_WIDTH];
    char prev[SCREEN_HEIGHT][SCREEN_WIDTH];
    int attrs[SCREEN_HEIGHT][SCREEN_WIDTH];
    int prevAttrs[SCREEN_HEIGHT][SCREEN_WIDTH];
    bool firstFrame;

    void syncPrev();
public:
    ScreenBuffer();
    void clear();
    void setChar(int x, int y, char c, int color = 0);
    void setAttr(int x, int y, int color);
    void drawString(int x, int y, const std::string& str, int color = 0);
    void drawHLine(int x, int y, int w, char c, int color = 0);
    void drawVLine(int x, int y, int h, char c, int color = 0);
    void drawBox(int x, int y, int w, int h, int color = 0);
    void drawBar(int x, int y, int w, int current, int max, int color = 0);
    void render();

    static void hideCursor();
    static void showCursor();
    static int getTerminalWidth();
    static int getTerminalHeight();
};

// ANSI color constants for use with ScreenBuffer
const int COL_DEFAULT  = 0;
const int COL_RED      = 31;
const int COL_GREEN    = 32;
const int COL_YELLOW   = 33;
const int COL_BLUE     = 34;
const int COL_MAGENTA  = 35;
const int COL_CYAN     = 36;
const int COL_WHITE    = 37;
const int COL_BRED     = 91;
const int COL_BGREEN   = 92;
const int COL_BYELLOW  = 93;
const int COL_BBLUE    = 94;
const int COL_BMAGENTA = 95;
const int COL_BCYAN    = 96;
const int COL_BWHITE   = 97;

class Renderer {
    ScreenBuffer& buf;
    int selOpt;
    std::string enemyName;
    int enemyHP, enemyMaxHP;
    std::string enemyArt[6];
    int playerHP, playerMaxHP;
    int playerMP, playerMaxMP;
    std::string playerName;
    std::string logMsg;
public:
    Renderer(ScreenBuffer& buffer);
    void setSelectedOption(int opt) { selOpt = opt; }
    void setEnemyInfo(const std::string& name, int hp, int maxHp, const std::string art[6]);
    void setPlayerInfo(const std::string& name, int hp, int maxHp, int mp, int maxMp);
    void setLogMessage(const std::string& msg) { logMsg = msg; }
    void drawBackground();
    void drawEnemy();
    void drawEnemyHealthBar();
    void drawCombatMenu();
    void drawPlayerInfo();
    void drawLog();
    void renderAll();
};

class InputHandler {
    int selectedOption;
public:
    InputHandler();
    int getSelectedOption() const { return selectedOption; }
    void setSelectedOption(int opt) { selectedOption = opt; }
    void moveUp();
    void moveDown();
};

class BattleSystem {
    BattleState currentState;
    Enemigo* currentEnemy;
    Jugador* player;
    ScreenBuffer screenBuffer;
    Renderer renderer;
    InputHandler inputHandler;
    bool battleOver;
    bool victory;
    bool fled;
    std::string enemyArt[6];
    std::string logMessage;

    void generateEnemyArt();
    void processInput();
    void doPlayerAction();
    void doEnemyTurn();
    void render();
    void setLog(const std::string& msg);
public:
    BattleSystem(Jugador& p, Enemigo& e);
    BattleState getState() const { return currentState; }
    bool isOver() const { return battleOver; }
    bool isVictory() const { return victory; }
    bool hasFled() const { return fled; }
    void run();
};

inline std::string dataPath(const std::string& filename) {
    return std::string("json/") + filename;
}

void limpiarBuffer();
void limpiarPantalla();

std::map<std::string, std::shared_ptr<Objeto>> cargarObjetosDesdeJSON(const std::string& archivo);
std::map<std::string, std::shared_ptr<Enemigo>> cargarEnemigosDesdeJSON(
    const std::string& archivo,
    const std::map<std::string, std::shared_ptr<Objeto>>& objetosDisponibles);
Jugador cargarHeroe(const std::string& archivo);
void guardarHeroe(const Jugador& jugador, const std::string& archivo);
std::shared_ptr<Enemigo> generarEnemigoPorNivel(
    const std::map<std::string, std::shared_ptr<Enemigo>>& enemigos, int nivelMaxPermitido);
void batalla(Jugador& jugador, const std::map<std::string, std::shared_ptr<Enemigo>>& enemigos);

#endif
