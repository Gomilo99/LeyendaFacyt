#ifndef INVENTARIO_HPP
#define INVENTARIO_HPP

#include "Batalla.hpp" // ScreenBuffer, colores
#include "Jugador.hpp"
#include "Objeto.hpp"
#include <vector>
#include <memory>

// Estados del inventario
enum class InvState {
    BROWSING,
    ITEM_ACTIONS,
    CONFIRM_ACTION,
    CLOSED
};

// Categorías de items
enum class ItemCategory {
    ARMAS,
    POCIONES,
    CLAVE
};

struct ItemEntry {
    std::string nombre;
    std::shared_ptr<Objeto> objeto;
    int cantidad;
};

class InvRenderer {
    private:
        ScreenBuffer &buf;
        int selectedIndex;  // Indice de la lista actual
        ItemCategory currentCategory;
        std::vector<ItemEntry> currentItems;  // Items filtrados por categoría
        std::string logMsg;

        // Info del jugador para el panel
        int playerHP, playerMaxHP;
        int playerMP, playerMaxMP;
        std::string playerName;
        int playerLevel;
        std::string equippedWeaponName;
        int equippedWeaponDmg;

    public:
        InvRenderer(ScreenBuffer &buffer) : buf(buffer) {}; // Constructor

        void setItems(const std::vector<ItemEntry> &items);
        void setSelectedIndex(int idx) { selectedIndex = idx; }
        void setCategory(ItemCategory cat) { currentCategory = cat; }
        void setPlayerInfo(int pHP, int pMaxHP, int pMP, int pMaxMP, std::string pName, 
            int pLevel, std::string eqWeaName, int eqWeaDmg){
            playerHP = pHP;
            playerMaxHP = pMaxHP;
            playerMP = pMP;
            playerMaxMP = pMaxMP;
            playerName = pName;
            playerLevel = pLevel;
            equippedWeaponName = equippedWeaponName;
            equippedWeaponDmg = eqWeaDmg;
        }
        void setLogMessage(const std::string &msg) { logMsg = msg; }

        void drawBackground();
        void drawCategoryTabs();
        void drawItemList();
        void drawItemDetails();
        void drawPlayerStats();
        void drawFooter();
        void renderAll();
};

class InventoryUI {
    private:
        InvState state;
        Jugador *player;
        ScreenBuffer screenBuffer;
        InvRenderer renderer;
        ItemCategory currentCategory;
        int selectedIndex;
        bool closed;
        std::string LogMessage;

        std::vector<ItemEntry> buildItemList(ItemCategory cat);
        void processInput(char key);
        void doAction();
        void render();
        void setLong(const std::string &msg);

    public:
        InventoryUI(Jugador &p) : player(&p) {};
        void run(); // bucle principal, igual que BattleSystem::run()
        bool isClosed() const { return closed; }

};
#endif