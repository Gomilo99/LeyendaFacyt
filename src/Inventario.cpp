#include "../lib/Inventario.hpp"
#include "../lib/Jugador.hpp"

/* Implementación de Métodos de InvRenderer - DRAW
- drawBakckground
- drawCategoryTabs
- drawItemList
- drawItemDetails
- drawPlayerStats
- renderALL
- render
*/

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
    int listX = 2, listY = 4, listW = 26, listH = 9;
    buf.drawBox(listX, listY, listW, listH, COL_CYAN);

    int visibleStart = 0;
    int visibleCount = listH - 2;
    if (selectedIndex >= visibleStart + visibleCount)
        visibleStart = selectedIndex - visibleCount + 1;

    // 1. Dibujar items primero
    for (int i = 0; i < visibleCount && i < (int)currentItems.size(); i++){
        int idx = visibleStart + i;
        if (idx < (int)currentItems.size()){
            bool selected = (idx == selectedIndex);
            int color = selected ? COL_BYELLOW : COL_CYAN;
            std::string prefix = selected ? " > " : "   "; // Tres espacios
            std::string line = prefix + currentItems[idx].nombre;
            if (currentItems[idx].cantidad > 1) line += " x" + std::to_string(currentItems[idx].cantidad);

            int maxW = listW - 3;
            // Se elimina maxW -2 porque el ultimo caracter debe ser el de borde y se añade uno nuevo al final
            if ((int)line.size() > maxW) line = line.substr(0, maxW - 2) + ".."; 

            buf.drawString(listX + 1, listY + 1 + (idx - visibleStart), line, color);
        }
    }

    // 2. Scroll indicators DESPUES, en el margen derecho (última columna de contenido)
    int arrowX = listX + listW - 2;  // última columna antes del borde |
    if (visibleStart > 0)
        buf.drawString(arrowX, listY + 1, "▲", COL_CYAN);
    if (visibleStart + visibleCount < (int)currentItems.size())
        buf.drawString(arrowX, listY + listH - 2, "▼", COL_CYAN);
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
        buf.drawString(detX + 2, detY + 4 + i, line, COL_DEFAULT); // detW - 4 por los bordes(2) y los espacios padding(2)
    }
}

void InvRenderer::drawPlayerStats(){
    int statX = 2, statY = 14, statW = SCREEN_WIDTH - 4, statH = 5;
    buf.drawBox(statX, statY, statW, statH, COL_CYAN);

    // Línea 1: Nombre + HP bar + MP bar
    buf.drawString(statX + 2, statY + 1, playerName, COL_BWHITE);

    int xCursor = statX + 2 + playerName.size() + 1;
    buf.drawString(xCursor, statY + 1, "HP", COL_GREEN);
    int hpPct = (playerHP > 0) ? (playerHP * 100 / playerMaxHP) : 0;
    int hpColor = (hpPct > 50 ) ? COL_GREEN : (hpPct > 25) ? COL_YELLOW : COL_RED;
    buf.drawBar(xCursor + 3, statY + 1, 10, playerHP, playerMaxHP, hpColor);

    std::string hpStr = std::to_string(playerHP) + "/" + std::to_string(playerMaxHP);
    buf.drawString(xCursor + 3 + 11, statY + 1, hpStr, hpColor);

    // MP a la derecha
    int mpX = statX + statW - 22;
    buf.drawString(mpX, statY + 1, "MP", COL_BLUE);
    buf.drawBar(mpX + 3, statY + 1, 10, playerMP, playerMaxMP, COL_BLUE);

    std::string mpStr = std::to_string(playerMP) + "/" + std::to_string(playerMaxMP);
    buf.drawString(mpX + 3 + 11, statY + 1, mpStr, COL_BLUE);

    // Línea 2: ATK, DEF, NIV
    std::string statsLine = "ATK: " + std::to_string(playerAtk) 
                            + " DEF: " + std::to_string(playerDef)
                            + " NIV: " + std::to_string(playerLevel);
    buf.drawString(statX + 2, statY + 2, statsLine, COL_WHITE);

    // Línea 3: EXP + ARMA
    std::string expStr = "EXP: " + std::to_string(playerExp)
                        + "/" + std::to_string(playerMaxExp);
    buf.drawString(statX + 2, statY + 3, expStr, COL_YELLOW);

    if (!equippedWeaponName.empty()) {
        std::string wpn = "ARMA: " + equippedWeaponName
                        + " (+" + std::to_string(equippedWeaponDmg) + ")";
        buf.drawString(statX + 2 + expStr.size() + 4, statY + 3, wpn, COL_YELLOW);
    }
}

void InvRenderer::drawFooter(){
    std::string footer = "[W/S] Navegar [A/D] Categoria [SPACE] OK [Q] Salir";
    int cx = SCREEN_WIDTH / 2;
    buf.drawString(cx - (int)footer.size() / 2, SCREEN_HEIGHT - 1, footer, COL_CYAN);
}

void InvRenderer::renderAll(){
    buf.clear();
    drawBackground();
    drawCategoryTabs();
    drawItemList();
    drawItemDetails();
    drawPlayerStats();
    drawFooter();
    if (!logMsg.empty()){
        buf.drawString(2, SCREEN_HEIGHT - 2, logMsg, COL_BYELLOW);
    }
    buf.render();
}

void InventoryUI::render(){
    // Pasar datos del jugador al renderer
    renderer.setPlayerInfo(
        player->getSalud(), player->getSaludMaxima(),
        player->getMana(), player->getManaMaxima(),
        player->getNombre(), player->getNivel(),
        player->getExperiencia(), player->getExperienciaNecesaria(),
        player->getAtaque(), player->getDefensa(),
        player->getArmaNombre(), player->getArmaDano()
    );
    renderer.setLogMessage(logMessage);
    renderer.renderAll();
}
/* Implementación de Métodos InventoryUI
- buildItemList
- processInput
- doAction
- render
- setLog
*/

std::vector<ItemEntry> InventoryUI::buildItemList(ItemCategory cat){
    std::vector<ItemEntry> result;
    const auto &objetos = player->getObjetosInventario();
    const auto &cantidades = player->getInventario();

    for (const auto &par : objetos){
        const std::string &nombre = par.first;
        auto obj = par.second;

        bool matches = false;
        switch (cat)
        {
            case ItemCategory::ARMAS:       matches = (obj->getTipo() == "Arma");       break;
            case ItemCategory::POCIONES:    matches = (obj->getTipo() == "Pocion");     break;
            case ItemCategory::CLAVE:       matches = (obj->getTipo() == "Objeto Clave"); break;
        }

        if (matches){
            int cant = 1;
            auto it = cantidades.find(nombre);
            if (it != cantidades.end()) cant = it->second;
            result.push_back({nombre, obj, cant});
        }
    }

    return result;
}

void InventoryUI::processInput(char key) {
    const auto& items = renderer.getItems();

    switch (state) {
    case InvState::BROWSING:
        if (items.empty()) break; // no hace nada si no hay items
        
        if (key == 'w' || key == 'W') {
            if (selectedIndex > 0) selectedIndex--;
            else selectedIndex = items.size() - 1; // circular
            renderer.setSelectedIndex(selectedIndex);
        } else if (key == 's' || key == 'S') {
            if (selectedIndex < (int)items.size() - 1) selectedIndex++;
            else selectedIndex = 0;
            renderer.setSelectedIndex(selectedIndex);
        } else if (key == 'a' || key == 'A') {
            // cambiar categoría anterior
            int cat = (int)currentCategory;
            cat = (cat - 1 + 3) % 3;
            currentCategory = (ItemCategory)cat;
            renderer.setCategory(currentCategory);
            renderer.setItems(buildItemList(currentCategory));
            selectedIndex = 0;
            renderer.setSelectedIndex(0);
        } else if (key == 'd' || key == 'D') {
            // cambiar categoría siguiente
            int cat = (int)currentCategory;
            cat = (cat + 1) % 3;
            currentCategory = (ItemCategory)cat;
            renderer.setCategory(currentCategory);
            renderer.setItems(buildItemList(currentCategory));
            selectedIndex = 0;
            renderer.setSelectedIndex(0);
        } else if (key == ' ') {
            if (!items.empty()) state = InvState::ITEM_ACTIONS;
        } else if (key == 'q' || key == 'Q') {
            state = InvState::CLOSED;
        }
        break;

    case InvState::ITEM_ACTIONS:
        // Submenú simple: [Usar] [Equipar] [Info] [Volver]
        // Se maneja igual que el menú de combate: w/s y space
        if (key == ' ') {
            doAction(); // ejecuta la acción seleccionada
            state = InvState::BROWSING;
        } else if (key == 'q' || key == 'Q' || key == 27) {
            state = InvState::BROWSING;
        }
        break;

    case InvState::CONFIRM_ACTION:
        if (key == 'w' || key == 's') {
            // toggle sí/no
        } else if (key == ' ') {
            // ejecutar
            state = InvState::BROWSING;
        } else if (key == 'q') {
            state = InvState::BROWSING;
        }
        break;
    }
}
void InventoryUI::doAction() {
    const auto &items = renderer.getItems();
    if (selectedIndex < 0 || selectedIndex >= (int)items.size()) return;

    auto &entry = items[selectedIndex];
    auto obj = entry.objeto;

    if (auto arma = std::dynamic_pointer_cast<Arma>(obj)){
        // Equipar Arma
        player->equiparArma(arma);
        logMessage = "Has equipado: " + arma->getNombre();
    }else if (auto pocion = std::dynamic_pointer_cast<Pocion>(obj)){
        // Usar pocion
        player->usarPocion(pocion.get());
        player->eliminarObjeto(entry.nombre);
        logMessage = "Has usado: " + pocion->getNombre();
        // Reconstruir lista porque se modificó el inventario
        renderer.setItems(buildItemList(currentCategory));
    }else{
        logMessage = obj->getDescripcion();
    }
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