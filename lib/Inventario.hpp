#ifndef INVENTARIO_HPP
#define INVENTARIO_HPP

/**
 * @file Inventario.hpp
 * Sistema de inventario con interfaz gráfica ASCII estilo combate.
 *
 * Proporciona dos clases principales:
 * - InvRenderer:  dibuja las secciones del inventario sobre un ScreenBuffer
 * - InventoryUI:  orquestador con máquina de estados, input y lógica de acciones
 *
 * Reutiliza ScreenBuffer (56x22, colores ANSI, redibujado diferencial)
 * y el mismo patrón de input que BattleSystem (navegación por teclado).
 */

#include "Batalla.hpp"
#include "Jugador.hpp"
#include "Objeto.hpp"
#include <vector>
#include <memory>

/**
 * @brief Estados de la máquina de estados del inventario.
 *
 * BROWSING:        navegando la lista de items (estado principal)
 * ITEM_ACTIONS:    submenú de acciones sobre el item seleccionado
 * CONFIRM_ACTION:  paso de confirmación antes de ejecutar (reservado)
 * CLOSED:          señal para salir del bucle de run()
 */
enum class InvState {
    BROWSING,
    ITEM_ACTIONS,
    CONFIRM_ACTION,
    CLOSED
};

/**
 * @brief Categorías de items para filtrar el inventario.
 *
 * Cada categoría corresponde a un tipo de Objeto::getTipo():
 * ARMAS    → "Arma"
 * POCIONES → "Pocion"
 * CLAVE    → "Objeto Clave"
 */
enum class ItemCategory {
    ARMAS,
    POCIONES,
    CLAVE
};

/**
 * @brief Entrada individual de la lista de items del inventario.
 *
 * Almacena la información necesaria para renderizar un item
 * en la lista scrolleable y ejecutar acciones sobre él.
 */
struct ItemEntry {
    std::string nombre;                  ///< Nombre del objeto (coincide con la clave en Jugador::objetosInventario)
    std::shared_ptr<Objeto> objeto;      ///< Puntero al objeto (Arma, Pocion u ObjClave)
    int cantidad;                        ///< Cantidad que posee el jugador de este objeto
};

/**
 * @brief Renderizador del frame de inventario.
 *
 * Dibuja cada sección del inventario sobre un ScreenBuffer usando los mismos
 * patrones que el Renderer de combate (drawBox, drawBar, drawString, colores ANSI).
 *
 * Las secciones son:
 * - drawBackground:    título + separador superior
 * - drawCategoryTabs:  pestañas Armas/Consumibles/Clave
 * - drawItemList:      lista scrolleable de items con indicadores ▲/▼
 * - drawItemDetails:   panel derecho con stats y descripción del item seleccionado
 * - drawPlayerStats:   panel inferior con HP, MP, ATK, DEF, NIV, EXP, arma equipada
 * - drawFooter:        controles en la última línea
 */
class InvRenderer {
    private:
        ScreenBuffer &buf;                   ///< Buffer de pantalla sobre el que se dibuja
        int selectedIndex;                   ///< Índice del item seleccionado en currentItems
        ItemCategory currentCategory;        ///< Categoría actual (determina color de pestaña)
        std::vector<ItemEntry> currentItems; ///< Items filtrados por categoría
        std::string logMsg;                  ///< Mensaje de log para la penúltima línea

        /// @name Stats del jugador (actualizadas cada frame por setPlayerInfo)
        ///@{
        int playerHP, playerMaxHP;
        int playerMP, playerMaxMP;
        std::string playerName;
        int playerLevel;
        int playerExp, playerMaxExp;
        int playerAtk, playerDef;
        std::string equippedWeaponName;
        int equippedWeaponDmg;
        ///@}

    public:
        /**
         * @brief Constructor. Almacena la referencia al ScreenBuffer.
         * @param buffer Referencia al ScreenBuffer sobre el que dibujar
         */
        InvRenderer(ScreenBuffer &buffer) : buf(buffer) {}; 

        /// @brief Devuelve la lista actual de items (const, para lectura en InventoryUI::processInput)
        const std::vector<ItemEntry>& getItems() const { return currentItems; }

        /// @brief Reemplaza la lista de items (ej. al cambiar de categoría)
        void setItems(const std::vector<ItemEntry> &items){ currentItems = items; }

        /// @brief Actualiza el índice seleccionado (lo recibe de InventoryUI tras navegar)
        void setSelectedIndex(int idx) { selectedIndex = idx; }

        /// @brief Cambia la categoría activa (cambia el color de la pestaña)
        void setCategory(ItemCategory cat) { currentCategory = cat; }

        /**
         * @brief Transfiere todas las stats del jugador al renderer.
         * 
         * Se llama desde InventoryUI::render() en cada frame para mantener
         * el panel de stats actualizado (HP, MP, EXP y arma pueden cambiar
         * después de usar una poción o equipar un arma).
         */
        void setPlayerInfo(int pHP, int pMaxHP, int pMP, int pMaxMP, std::string pName, 
            int pLevel, int pExp, int pMaxExp, int pAtk, int pDef, std::string eqWeaName, int eqWeaDmg){
            playerHP = pHP;
            playerMaxHP = pMaxHP;
            playerMP = pMP;
            playerMaxMP = pMaxMP;
            playerName = pName;
            playerLevel = pLevel;
            playerExp = pExp;
            playerMaxExp = pMaxExp;
            playerAtk = pAtk;
            playerDef = pDef;
            equippedWeaponName = eqWeaName;
            equippedWeaponDmg = eqWeaDmg;
        }

        /// @brief Establece el mensaje de log que se muestra en la penúltima línea
        void setLogMessage(const std::string &msg) { logMsg = msg; }

        /// @name Métodos de dibujo (cada uno renderiza una sección del frame)
        ///@{
        void drawBackground();       ///< Línea 0: título del inventario
        void drawCategoryTabs();     ///< Línea 2: pestañas de categoría
        void drawItemList();         ///< Líneas 4-11: lista scrolleable de items
        void drawItemDetails();      ///< Líneas 4-11: panel derecho con info del item
        void drawPlayerStats();      ///< Líneas 14-16: panel de stats del jugador
        void drawFooter();           ///< Línea 21: controles
        ///@}

        /**
         * @brief Compone el frame completo y lo envía a la terminal.
         *
         * Orden: clear → drawBackground → drawCategoryTabs → drawItemList →
         * drawItemDetails → drawPlayerStats → drawFooter → logMsg → buf.render()
         */
        void renderAll();
};

/**
 * @brief Orquestador del inventario con máquina de estados.
 *
 * Sigue el mismo patrón que BattleSystem:
 * - Crea su propio ScreenBuffer (independiente del de combate)
 * - Bucle principal run(): render → input → processInput → loop
 * - Tres estados: BROWSING (navegación), ITEM_ACTIONS (ejecución), CLOSED (salida)
 *
 * Integración:
 * - En combate: se crea una instancia en doPlayerAction() case 2,
 *   se ejecuta run(), y al salir se llama a screenBuffer.forceRedraw()
 * - En overworld: se crea en GameManager::mostrarInventario(),
 *   se ejecuta run(), y al salir se hace limpiarPantalla() + renderMapa()
 */
class InventoryUI {
    private:
        InvState state;                    ///< Estado actual de la máquina de estados
        Jugador *player;                   ///< Puntero al jugador (no owned, referencia externa)
        ScreenBuffer screenBuffer;         ///< Buffer propio (independiente del de combate)
        InvRenderer renderer;              ///< Renderizador asociado a screenBuffer
        ItemCategory currentCategory;      ///< Categoría seleccionada en las pestañas
        int selectedIndex;                 ///< Índice del item seleccionado en la lista actual
        bool closed;                       ///< Flag de cierre (equivalente a state == CLOSED)
        std::string logMessage;            ///< Mensaje de log para el frame actual

        /**
         * @brief Construye la lista de items filtrada por categoría.
         *
         * Recorre Jugador::objetosInventario y filtra por Objeto::getTipo()
         * contra la categoría indicada. Busca la cantidad en Jugador::inventario.
         * 
         * @param cat Categoría por la que filtrar (ARMAS, POCIONES, CLAVE)
         * @return Vector de ItemEntry con nombre, objeto y cantidad
         */
        std::vector<ItemEntry> buildItemList(ItemCategory cat);

        /**
         * @brief Procesa una tecla según el estado actual.
         *
         * BROWSING:        w/s navegan, a/d cambian categoría, space acciones, q salir
         * ITEM_ACTIONS:    space ejecuta, q vuelve
         * CONFIRM_ACTION:  (reservado)
         */
        void processInput(char key);

        /**
         * @brief Ejecuta la acción correspondiente al tipo del item seleccionado.
         *
         * - Arma:   equiparArma()
         * - Pocion: usarPocion() + eliminarObjeto() + reconstruir lista
         * - Otro:   mostrar descripción
         */
        void doAction();

        /**
         * @brief Renderiza un frame completo del inventario.
         *
         * Lee todas las stats del jugador, las pasa al renderer
         * y llama a renderer.renderAll().
         */
        void render();

        /// @brief Actualiza el mensaje de log interno
        void setLog(const std::string &msg){ logMessage = msg; }

    public:
        /**
         * @brief Constructor. Inicializa player y renderer.
         * 
         * Nota: screenBuffer se construye antes que renderer (orden de declaración),
         * por lo que renderer recibe una referencia válida.
         * 
         * @param p Referencia al jugador (no se copia, se almacena puntero)
         */
        InventoryUI(Jugador &p) : player(&p), renderer(screenBuffer) {};

        /**
         * @brief Bucle principal del inventario.
         *
         * 1. Oculta el cursor de la terminal
         * 2. Inicializa estado, categoría e índice
         * 3. Bucle: render() → Platform::getKey() → processInput() hasta CLOSED
         * 4. Muestra el cursor
         *
         * Es síncrono: bloquea hasta que el usuario cierra el inventario con Q.
         */
        void run();

        /// @brief Indica si el inventario se cerró (útil después de run())
        bool isClosed() const { return closed; }

};
#endif