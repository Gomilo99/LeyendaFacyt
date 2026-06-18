#include "../lib/Inventario.hpp"
#include "../lib/Jugador.hpp"

void InvRenderer::drawBackground(){
    buf.drawString(0, 0, "=== LEYENDA DEL CAMPUS - INVENTARIO ===", COL_BYELLOW);
    buf.drawHLine(0, 1, SCREEN_WIDTH, '-', COL_CYAN);
}
void InvRenderer::drawCategoryTabs(){
    const char *names[] = { "Armas", "Consumibles", "Clave" };
    int x = 4;
    for (int i = 0; i < 3; i++){
        std::string tab = "[" + std::string(names[i]) + "]";
        int color = (i == (int)currentCategory) ? COL_BYELLOW : COL_CYAN;
        buf.drawString(x, 2, tab, color);
        x += tab.size() + 3;
    }
}

void InvRenderer::drawItemList(){
    // Dibujar recuadro de objetos
    int listX = 2, listY = 4, listW = 26, listH = 9;
    buf.drawBox(listX, listY, listW, listH, COL_CYAN);

    int visibleStart = 0;
    int visibleCount = listH - 2; // 7 items visibles
    if (selectedIndex >= visibleStart + visibleCount)
        visibleStart = selectedIndex - visibleCount + 1;

    for (int i = 0; i < visibleCount && i < (int)currentItems.size(); i++){
        int idx = visibleStart + i;
        //if (idx >= (int)currentItems.size()) break;
        // Se aplica lógica inversa
        if (idx < (int)currentItems.size()){
            bool selected = (idx == selectedIndex);
            int color = selected ? COL_BYELLOW : COL_CYAN;
            std::string prefix = selected ? " > " : "   "; // Tres espacios
            std::string line = prefix + currentItems[idx].nombre;
            if (currentItems[idx].cantidad > 1) line += " x" + std::to_string(currentItems[idx].cantidad);

            // Truncar si es muy larga
            int maxW = listW - 3;
            // Se elimina maxW -2 porque el ultimo caracter debe ser el de borde y se añade uno nuevo al final
            if ((int)line.size() > maxW) line = line.substr(0, maxW - 2) + ".."; 

            buf.drawString(listX + 1, listY + 1 + (idx - visibleStart), line, color);
        }
    }

    // Indicaciones de scroll
    if (visibleStart > 0)
        buf.drawString(listX + 1, listY + 1, "▲", COL_CYAN);
    if (visibleStart + visibleCount < (int)currentItems.size())
        buf.drawString(listX + 1, listY + listH - 2, "▼", COL_CYAN);
}

// Panel Derecho (26x9)
void InvRenderer::drawItemDetails() {
    int detX = 29, detY = 4, detW = 25, detH = 9;
    buf.drawBox(detX, detY, detW, detH, COL_CYAN);

    if (selectedIndex < 0 || selectedIndex >= (int)currentItems.size()) return;

    auto &entry = currentItems[selectedIndex];
    auto obj = entry.objeto;

    // Nombre
    buf.drawString(detX + 2, detY + 1, obj->getNombre(), COL_WHITE);

    // Info según tipo
    if (auto arma = std::dynamic_pointer_cast<Arma>(obj)){
        std::string dmgStr = "Daño: +" + std::to_string(arma->getDano());
        buf.drawString(detX + 2, detY + 2, dmgStr, COL_YELLOW);
    }
    if (auto consumible = std::dynamic_pointer_cast<Pocion>(obj)){
        std::string healStr = "Cura: +" + std::to_string(consumible->getCuracion()) + " HP";
        buf.drawString(detX + 2, detY + 2, healStr, COL_GREEN);
    }

    // Descripción (wrap manual o truncado)
    std::string desc = obj->getDescripcion();
    for (int i = 0; i < 3 && (int)desc.size() > i * (detW - 4); i++){
        std::string line = desc.substr(i * (detW - 4), detW - 4); // Falta explicación de por que se resta esos números y por que i debe ser menor que 3
        buf.drawString(detX + 2, detY + 4 + i, line, COL_DEFAULT);
    }
}

void InvRenderer::drawPlayerStats(){
    int statX = 2, statY = 14, statW = SCREEN_WIDTH - 4, statH = 3;
    buf.drawBox(statX, statY, statW, statH, COL_CYAN);

    // Nombre del jugador
    buf.drawString(statX + 2, statY + 1, playerName, COL_BWHITE);

    // Barra de HP
    buf.drawString(statX + 2 + playerName.size() + 1, statY + 1, "HP", COL_GREEN);
    int hpPct = (playerMaxHP > 0) ? (playerHP * 100 / playerMaxHP) : 0;
    int hpColor = (hpPct > 50) ? COL_GREEN : (hpPct > 25) ? COL_YELLOW : COL_RED;
    buf.drawBar(statX + 2 + playerName.size() + 4, statY + 1, 12, playerHP, playerMaxHP, hpColor);

    std::string hpStr = std::to_string(playerHP) + "/" + std::to_string(playerMaxHP);
    buf.drawString(statX + 2 + playerName.size() + 4 + 13, statY + 1, hpStr, hpColor);

    // Arma Equipada
    if(!equippedWeaponName.empty()){
        std::string wpn = "Arma: " + equippedWeaponName + "(+" + std::to_string(equippedWeaponDmg) + ")";
        buf.drawString(statX + 2, statY + 2, wpn, COL_YELLOW);
    }
    // NOTA: Falta Representar el resto de estadisticas
}

void InventoryUI::processInput(char key){

}

void InventoryUI::run(){
    ScreenBuffer::hideCursor();
    state = InvState::BROWSING;
    currentCategory = ItemCategory::ARMAS;
    selectedIndex = 0;
    closed = false;

    renderer.setItems(buildItemList(currentCategory));
    renderer.setSelectedIndex(0);
    setLog("Selecciona un objeto (W/S, SPACE)");

    while(state != InvState::CLOSED){
        render();

        char key;
        if(std::cin.get(key)){
            processInput(key);
        }
    }

    ScreenBuffer::showCursor();
}