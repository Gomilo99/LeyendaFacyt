#ifndef BATALLA_HPP
#define BATALLA_HPP

#include <string>
#include <vector>
#include <sstream>
#include "Enemigo.hpp"
#include "Jugador.hpp"

// Dimensiones fijas del buffer de pantalla de combate
const int SCREEN_WIDTH = 68;
const int SCREEN_HEIGHT = 25;

// Maquina de estados del combate por turnos
enum class BattleState {
    PLAYER_TURN,   // Esperando entrada del jugador (W/S/SPACE)
    PLAYER_ACTION, // Ejecutando la accion seleccionada
    ENEMY_TURN,    // Turno del enemigo (con pausa animada)
    ANIMATION,     // Reservado para animaciones futuras
    VICTORY,       // Enemigo derrotado
    DEFEAT,        // Jugador sin HP
    FLEE           // Jugador huyo del combate
};

// Buffer de pantalla con doble capa (chars + colores) y redibujado diferencial
class ScreenBuffer {
    // Capa de caracteres visibles
    char grid[SCREEN_HEIGHT][SCREEN_WIDTH];
    // Capa anterior para detectar cambios linea por linea
    char prev[SCREEN_HEIGHT][SCREEN_WIDTH];
    // Codigos de color ANSI por celda (0 = default)
    int attrs[SCREEN_HEIGHT][SCREEN_WIDTH];
    // Atributos anteriores para detectar cambios
    int prevAttrs[SCREEN_HEIGHT][SCREEN_WIDTH];
    // true = forzar redibujado completo en el proximo render()
    bool firstFrame;

    // Copia grid+attrs a prev+prevAttrs para el proximo frame
    void syncPrev();
public:
    ScreenBuffer();

    // Llena todo el buffer con espacios y resetea colores
    void clear();
    // Escribe un caracter en (x,y) con color ANSI opcional
    void setChar(int x, int y, char c, int color = 0);
    // Cambia solo el color de una celda sin tocar el caracter
    void setAttr(int x, int y, int color);
    // Dibuja texto horizontal desde (x,y)
    void drawString(int x, int y, const std::string& str, int color = 0);
    // Dibuja linea horizontal de longitud w con el caracter c
    void drawHLine(int x, int y, int w, char c, int color = 0);
    // Dibuja linea vertical de altura h con el caracter c
    void drawVLine(int x, int y, int h, char c, int color = 0);
    // Dibuja un recuadro de w x h
    void drawBox(int x, int y, int w, int h, int color = 0);
    // Dibuja una barra de progreso: current/max de ancho w
    void drawBar(int x, int y, int w, int current, int max, int color = 0);
    // Vuelca el buffer a la terminal: solo lineas modificadas
    void render();
    // Marca el buffer como invalido para forzar redibujado completo
    void forceRedraw();

    // Oculta el cursor de la terminal (\033[?25l)
    static void hideCursor();
    // Muestra el cursor de la terminal (\033[?25h)
    static void showCursor();
    // Devuelve el ancho de la terminal (Win32) o 80 como fallback
    static int getTerminalWidth();
    // Devuelve el alto de la terminal (Win32) o 24 como fallback
    static int getTerminalHeight();
};

// Constantes de color ANSI para drawString, drawBar, etc.
const int COL_DEFAULT  = 0;
const int COL_RED      = 31;
const int COL_GREEN    = 32;
const int COL_YELLOW   = 33;
const int COL_BLUE     = 34;
const int COL_MAGENTA  = 35;
const int COL_CYAN     = 36;
const int COL_WHITE    = 37;
const int COL_BRED     = 91;   // rojo brillante
const int COL_BGREEN   = 92;
const int COL_BYELLOW  = 93;
const int COL_BBLUE    = 94;
const int COL_BMAGENTA = 95;
const int COL_BCYAN    = 96;
const int COL_BWHITE   = 97;

// Renderizador del frame de combate. Dibuja cada seccion sobre un ScreenBuffer.
class Renderer {
    ScreenBuffer& buf;      // buffer sobre el que se dibuja
    int selOpt;             // opcion del menu seleccionada (0-3)
    std::string enemyName;
    int enemyHP, enemyMaxHP;
    std::string enemyArt[6]; // 6 lineas de arte ASCII del enemigo
    int playerHP, playerMaxHP;
    int playerMP, playerMaxMP;
    std::string playerName;
    std::string logMsg;     // mensaje de estado en la linea inferior
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
    void renderAll();       // limpia buffer y dibuja todo el frame
};

// Maneja la navegacion por el menu de 4 opciones (W/S, seleccion circular)
class InputHandler {
    int selectedOption;     // 0=Atacar, 1=Magia, 2=Inventario, 3=Huir
public:
    InputHandler();
    int getSelectedOption() const { return selectedOption; }
    void setSelectedOption(int opt) { selectedOption = opt; }
    void moveUp();
    void moveDown();
};

// Maquina de estados del combate. Coordina ScreenBuffer, Renderer e InputHandler.
class BattleSystem {
    BattleState currentState;
    Enemigo* currentEnemy;  // puntero al enemigo (no owned, referencia externa)
    Jugador* player;        // puntero al jugador (no owned, referencia externa)
    ScreenBuffer screenBuffer;
    Renderer renderer;
    InputHandler inputHandler;
    bool battleOver;
    bool victory;
    bool fled;
    std::string enemyArt[6];
    std::string logMessage;

    // Supresion de cout para evitar que atacar/recibirDano ensucien la terminal
    std::streambuf* oldCoutBuf;
    std::ostringstream coutSink;
    void suppressCout();
    void restoreCout();

    // Lee W/S/SPACE del stdin durante PLAYER_TURN
    void processInput();
    // Ejecuta la opcion seleccionada del menu
    void doPlayerAction();
    // Turno del enemigo: ataca al jugador con pausa animada
    void doEnemyTurn();
    // Compone y envía el frame completo a la terminal
    void render();
    // Actualiza el mensaje de log que se muestra en pantalla
    void setLog(const std::string& msg);
public:
    BattleSystem(Jugador& p, Enemigo& e);
    BattleState getState() const { return currentState; }
    bool isOver() const { return battleOver; }
    bool isVictory() const { return victory; }
    bool hasFled() const { return fled; }
    // Bucle principal del combate: PLAYER_TURN → accion → ENEMY_TURN → loop
    void run();
};

// Limpia el buffer de entrada (cin.clear + ignore hasta \n)
void limpiarBuffer();
// Limpia la terminal (\033[2J\033[1;1H)
void limpiarPantalla();

// Punto de entrada al combate: muestra intro, instancia BattleSystem, maneja loot/exp post-batalla
void batalla(Jugador& jugador, Enemigo& enemigo);

#endif
