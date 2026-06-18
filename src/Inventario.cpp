#include "../lib/Inventario.hpp"
#include "../lib/Jugador.hpp"
#include "../lib/Platform.hpp"

/**
 * @file Inventario.cpp
 * Implementación del sistema de inventario con interfaz gráfica ASCII.
 *
 * Sigue la misma arquitectura que batalla.cpp:
 * - InvRenderer: siete funciones de dibujo que componen el frame
 * - InventoryUI: orquestador con máquina de estados, buildItemList,
 *   processInput, doAction, render y run
 */

// ==================== INV RENDERER ====================

/**
 * Dibuja la línea 0 del frame: título del inventario en amarillo brillante
 * y la línea 1: separador de guiones en cian.
 *
 * Es idéntico en estructura a Renderer::drawBackground() del combate,
 * solo cambia el texto del título.
 */
void InvRenderer::drawBackground(){
    buf.drawString(0, 0, "=== LEYENDA DEL CAMPUS - INVENTARIO ===", COL_BYELLOW);
    buf.drawHLine(0, 1, SCREEN_WIDTH, '-', COL_CYAN);
}

/**
 * Dibuja las tres pestañas de categoría en la línea 2.
 *
 * La pestaña activa (currentCategory) se pinta en amarillo brillante,
 * las inactivas en cian. La posición horizontal se calcula dinámicamente
 * sumando el ancho de cada pestaña más 3 espacios de separación.
 *
 * Las categorías son: Armas, Consumibles, Clave.
 * Se navegan con las teclas A/D en el estado BROWSING.
 */
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

/**
 * Dibuja el panel izquierdo (26x9) con la lista scrolleable de items.
 *
 * Scroll:
 * - Solo se muestran listH - 2 = 7 items visibles
 * - visibleStart se desplaza cuando selectedIndex sale del rango visible
 * - Si currentItems está vacío, muestra "Sin objetos.." centrado en magenta
 *
 * Formato de cada línea:
 *   " > nombre xN" para el item seleccionado (amarillo brillante)
 *   "   nombre xN" para los no seleccionados (cian)
 *
 * Truncado: si la línea supera listW - 3 = 23 caracteres, se corta a
 * maxW - 2 = 21 caracteres y se añade ".." al final.
 *
 * Scroll indicators ▲/▼: se dibujan DESPUÉS de los items en la última
 * columna de contenido (listX + listW - 2) para evitar solapamiento.
 * ▲ aparece solo si visibleStart > 0, ▼ si hay más items después del final.
 */
void InvRenderer::drawItemList(){
    int listX = 2, listY = 4, listW = 26, listH = 9;
    buf.drawBox(listX, listY, listW, listH, COL_CYAN);

    int visibleStart = 0;
    int visibleCount = listH - 2;
    if (selectedIndex >= visibleStart + visibleCount)
        visibleStart = selectedIndex - visibleCount + 1;

    if (currentItems.empty()){
        std::string msg = "Sin objetos..";
        buf.drawString(listX + (listW - msg.size()) / 2, listY + listH / 2, msg, COL_MAGENTA);
        return;
    }

    for (int i = 0; i < visibleCount && i < (int)currentItems.size(); i++){
        int idx = visibleStart + i;
        if (idx < (int)currentItems.size()){
            bool selected = (idx == selectedIndex);
            int color = selected ? COL_BYELLOW : COL_CYAN;
            std::string prefix = selected ? " > " : "   ";
            std::string line = prefix + currentItems[idx].nombre;
            if (currentItems[idx].cantidad > 1) line += " x" + std::to_string(currentItems[idx].cantidad);

            int maxW = listW - 3;
            if ((int)line.size() > maxW) line = line.substr(0, maxW - 2) + "..";

            buf.drawString(listX + 1, listY + 1 + (idx - visibleStart), line, color);
        }
    }

    int arrowX = listX + listW - 2;
    if (visibleStart > 0)
        buf.drawString(arrowX, listY + 1, "▲", COL_CYAN);
    if (visibleStart + visibleCount < (int)currentItems.size())
        buf.drawString(arrowX, listY + listH - 2, "▼", COL_CYAN);
}

/**
 * Dibuja el panel derecho (37x9) con la información detallada del item
 * actualmente seleccionado en la lista.
 *
 * Usa dynamic_pointer_cast para determinar el tipo real del objeto:
 * - Arma:   muestra "Daño: +X" en amarillo
 * - Pocion: muestra "Cura: +X HP" en verde
 * - Otro:   sección de tipo vacía (solo nombre + descripción)
 *
 * La descripción se wrappea manualmente en hasta 3 líneas de
 * detW - 4 = 33 caracteres cada una (detW menos 2 bordes y 2 paddings).
 *
 * Si selectedIndex está fuera de rango o la lista vacía, retorna
 * sin dibujar nada (el panel queda vacío pero el box ya está dibujado).
 */
void InvRenderer::drawItemDetails() {
    int detX = 29, detY = 4, detW = 37, detH = 9;
    buf.drawBox(detX, detY, detW, detH, COL_CYAN);

    if (selectedIndex < 0 || selectedIndex >= (int)currentItems.size()) return;

    auto &entry = currentItems[selectedIndex];
    auto obj = entry.objeto;

    buf.drawString(detX + 2, detY + 1, obj->getNombre(), COL_WHITE);

    if (auto arma = std::dynamic_pointer_cast<Arma>(obj)){
        std::string dmgStr = "Daño: +" + std::to_string(arma->getDano());
        buf.drawString(detX + 2, detY + 2, dmgStr, COL_YELLOW);
    }
    if (auto consumible = std::dynamic_pointer_cast<Pocion>(obj)){
        std::string healStr = "Cura: +" + std::to_string(consumible->getCuracion()) + " HP";
        buf.drawString(detX + 2, detY + 2, healStr, COL_GREEN);
    }

    std::string desc = obj->getDescripcion();
    for (int i = 0; i < 3 && (int)desc.size() > i * (detW - 4); i++){
        std::string line = desc.substr(i * (detW - 4), detW - 4);
        buf.drawString(detX + 2, detY + 4 + i, line, COL_DEFAULT);
    }
}

/**
 * Dibuja el panel inferior (52x5) con las estadísticas completas del jugador.
 *
 * Línea 1 (statY + 1):
 *   - Nombre del jugador en blanco brillante
 *   - Barra de HP (10 chars) con color dinámico según %:
 *     verde > 50%, amarillo > 25%, rojo ≤ 25%
 *   - Valor numérico HP: "X/Y"
 *   - Barra de MP (10 chars) en azul fijo
 *   - Valor numérico MP: "X/Y"
 *
 * Línea 2 (statY + 2):
 *   - "ATK: X  DEF: X  NIV: X" en blanco
 *
 * Línea 3 (statY + 3):
 *   - "EXP: X/Y" en amarillo
 *   - "ARMA: nombre (+X)" en amarillo (solo si hay arma equipada)
 *
 * Las stats se actualizan cada frame desde InventoryUI::render().
 */
void InvRenderer::drawPlayerStats(){
    int statX = 2, statY = 14, statW = SCREEN_WIDTH - 4, statH = 5;
    buf.drawBox(statX, statY, statW, statH, COL_CYAN);

    buf.drawString(statX + 2, statY + 1, playerName, COL_BWHITE);

    int xCursor = statX + 2 + playerName.size() + 1;
    buf.drawString(xCursor, statY + 1, "HP", COL_GREEN);
    int hpPct = (playerHP > 0) ? (playerHP * 100 / playerMaxHP) : 0;
    int hpColor = (hpPct > 50 ) ? COL_GREEN : (hpPct > 25) ? COL_YELLOW : COL_RED;
    buf.drawBar(xCursor + 3, statY + 1, 10, playerHP, playerMaxHP, hpColor);

    std::string hpStr = std::to_string(playerHP) + "/" + std::to_string(playerMaxHP);
    buf.drawString(xCursor + 3 + 11, statY + 1, hpStr, hpColor);

    int mpX = statX + statW - 22;
    buf.drawString(mpX, statY + 1, "MP", COL_BLUE);
    buf.drawBar(mpX + 3, statY + 1, 10, playerMP, playerMaxMP, COL_BLUE);

    std::string mpStr = std::to_string(playerMP) + "/" + std::to_string(playerMaxMP);
    buf.drawString(mpX + 3 + 11, statY + 1, mpStr, COL_BLUE);

    std::string statsLine = "ATK: " + std::to_string(playerAtk) 
                            + " DEF: " + std::to_string(playerDef)
                            + " NIV: " + std::to_string(playerLevel);
    buf.drawString(statX + 2, statY + 2, statsLine, COL_WHITE);

    std::string expStr = "EXP: " + std::to_string(playerExp)
                        + "/" + std::to_string(playerMaxExp);
    buf.drawString(statX + 2, statY + 3, expStr, COL_YELLOW);

    if (!equippedWeaponName.empty()) {
        std::string wpn = "ARMA: " + equippedWeaponName
                        + " (+" + std::to_string(equippedWeaponDmg) + ")";
        buf.drawString(statX + 2 + expStr.size() + 4, statY + 3, wpn, COL_YELLOW);
    }
}

/**
 * Dibuja la línea 21 (la última del buffer): los controles del inventario
 * centrados horizontalmente en cian.
 *
 * Texto: "[W/S] Navegar [A/D] Categoria [SPACE] OK [Q] Salir"
 */
void InvRenderer::drawFooter(){
    std::string footer = "[W/S] Navegar [A/D] Categoria [SPACE] OK [Q] Salir";
    int cx = SCREEN_WIDTH / 2;
    buf.drawString(cx - (int)footer.size() / 2, SCREEN_HEIGHT - 1, footer, COL_CYAN);
}

/**
 * Compone el frame completo del inventario y lo envía a la terminal.
 *
 * Orden de dibujo:
 *   1. clear() — limpia todo el buffer
 *   2. drawBackground() — título + separador
 *   3. drawCategoryTabs() — pestañas
 *   4. drawItemList() — lista de items
 *   5. drawItemDetails() — detalle del item seleccionado
 *   6. drawPlayerStats() — stats del jugador
 *   7. drawFooter() — controles
 *   8. logMsg — mensaje de estado (si no está vacío)
 *   9. buf.render() — vuelca a terminal con redibujado diferencial
 */
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

// ==================== INVENTORY UI ====================

/**
 * Renderiza un frame completo del inventario.
 *
 * Lee todas las stats actuales del jugador y las transfiere al InvRenderer
 * mediante setPlayerInfo(). Luego establece el mensaje de log y delega
 * el dibujo completo a InvRenderer::renderAll().
 *
 * Se llama en cada iteración del bucle principal (run()), garantizando
 * que los datos del panel de stats estén siempre actualizados (HP, MP,
 * EXP y arma pueden cambiar tras usar una poción o equipar un arma).
 */
void InventoryUI::render(){
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

/**
 * Construye la lista de items filtrada por categoría.
 *
 * Recorre Jugador::getObjetosInventario() (un map<string, shared_ptr<Objeto>>)
 * y filtra cada entrada comparando Objeto::getTipo() con la categoría:
 *   - "Arma"        → ARMAS
 *   - "Pocion"      → POCIONES
 *   - "Objeto Clave" → CLAVE
 *
 * Para cada coincidencia, busca la cantidad en Jugador::getInventario()
 * (el map<string, int> paralelo que lleva el conteo).
 *
 * @param cat Categoría por la que filtrar
 * @return Vector de ItemEntry listo para pasar a InvRenderer::setItems()
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

/**
 * Procesa una tecla según el estado actual de la máquina de estados.
 *
 * Estados y teclas:
 *
 * BROWSING:
 *   Q → Cierra el inventario (state = CLOSED). Se chequea ANTES que
 *       items.empty() para permitir salir aunque no haya objetos.
 *   W → Navega hacia arriba en la lista (circular: índice 0 → último).
 *       No hace nada si la lista está vacía.
 *   S → Navega hacia abajo (circular: último → índice 0).
 *       No hace nada si la lista está vacía.
 *   A → Cambia a la categoría anterior ((cat - 1 + 3) % 3).
 *       Reconstruye la lista con buildItemList(), resetea selectedIndex.
 *   D → Cambia a la categoría siguiente ((cat + 1) % 3). Igual que A.
 *   SPACE → Si hay items, pasa a ITEM_ACTIONS.
 *
 * ITEM_ACTIONS:
 *   SPACE → Ejecuta la acción del item (doAction()), vuelve a BROWSING.
 *   Q / ESC → Vuelve a BROWSING sin ejecutar nada.
 *
 * CONFIRM_ACTION:
 *   (Reservado para futura confirmación sí/no antes de usar/descartar)
 *
 * @param key Caracter de la tecla presionada (desde Platform::getKey())
 */
void InventoryUI::processInput(char key) {
    const auto& items = renderer.getItems();

    switch (state) {
    case InvState::BROWSING:
        if (key == 'q' || key == 'Q') {
            state = InvState::CLOSED;
            break;
        }

        if (key == 'w' || key == 'W') {
            if (!items.empty()) {
                if (selectedIndex > 0) selectedIndex--;
                else selectedIndex = items.size() - 1;
                renderer.setSelectedIndex(selectedIndex);
            }
        } else if (key == 's' || key == 'S') {
            if (!items.empty()) {
                if (selectedIndex < (int)items.size() - 1) selectedIndex++;
                else selectedIndex = 0;
                renderer.setSelectedIndex(selectedIndex);
            }

        } else if (key == 'a' || key == 'A') {
            int cat = (int)currentCategory;
            cat = (cat - 1 + 3) % 3;
            currentCategory = (ItemCategory)cat;
            renderer.setCategory(currentCategory);
            renderer.setItems(buildItemList(currentCategory));
            selectedIndex = 0;
            renderer.setSelectedIndex(0);

        } else if (key == 'd' || key == 'D') {
            int cat = (int)currentCategory;
            cat = (cat + 1) % 3;
            currentCategory = (ItemCategory)cat;
            renderer.setCategory(currentCategory);
            renderer.setItems(buildItemList(currentCategory));
            selectedIndex = 0;
            renderer.setSelectedIndex(0);

        } else if (key == ' ') {
            if (!items.empty()) state = InvState::ITEM_ACTIONS;
        }

        break;

    case InvState::ITEM_ACTIONS:
        if (key == ' ') {
            doAction();
            state = InvState::BROWSING;
        } else if (key == 'q' || key == 'Q' || key == 27) {
            state = InvState::BROWSING;
        }
        break;

    case InvState::CONFIRM_ACTION:
        if (key == 'w' || key == 's') {
        } else if (key == ' ') {
            state = InvState::BROWSING;
        } else if (key == 'q') {
            state = InvState::BROWSING;
        }
        break;
    default: break;
    }
}

/**
 * Ejecuta la acción correspondiente al tipo del item seleccionado.
 *
 * Usa dynamic_pointer_cast para determinar el tipo real:
 *
 * - Arma (dynamic_pointer_cast<Arma> exitoso):
 *     Llama a Jugador::equiparArma() que actualiza el ataque sumando
 *     el daño del arma nueva y restando el de la anterior.
 *     Mensaje: "Has equipado: <nombre>"
 *
 * - Pocion (dynamic_pointer_cast<Pocion> exitoso):
 *     Llama a Jugador::usarPocion(Objeto*) que hace dynamic_cast<Pocion*>
 *     internamente y cura al jugador.
 *     Luego Jugador::eliminarObjeto() decrementa la cantidad (o elimina
 *     la entrada si llega a 0).
 *     Finalmente reconstruye la lista con buildItemList() porque el
 *     inventario cambió (el item pudo desaparecer).
 *     Mensaje: "Has usado: <nombre>"
 *
 * - Otro (ObjClave, etc.):
 *     Muestra la descripción del objeto en el log.
 *     Mensaje: descripción textual.
 *
 * Si selectedIndex está fuera de rango, retorna sin hacer nada.
 */
void InventoryUI::doAction() {
    const auto &items = renderer.getItems();
    if (selectedIndex < 0 || selectedIndex >= (int)items.size()) return;

    auto &entry = items[selectedIndex];
    auto obj = entry.objeto;

    if (auto arma = std::dynamic_pointer_cast<Arma>(obj)){
        player->equiparArma(arma);
        logMessage = "Has equipado: " + arma->getNombre();
    }else if (auto pocion = std::dynamic_pointer_cast<Pocion>(obj)){
        player->usarPocion(pocion.get());
        player->eliminarObjeto(entry.nombre);
        logMessage = "Has usado: " + pocion->getNombre();
        renderer.setItems(buildItemList(currentCategory));
    }else{
        logMessage = obj->getDescripcion();
    }
}

/**
 * Bucle principal del inventario.
 *
 * Flujo:
 *   1. Oculta el cursor de la terminal (ScreenBuffer::hideCursor())
 *   2. Inicializa estado a BROWSING, categoría a ARMAS, índice a 0
 *   3. Construye la lista inicial con buildItemList(ARMAS)
 *   4. Bucle while(state != CLOSED):
 *        a. render() — dibuja el frame completo con stats actualizadas
 *        b. Platform::getKey() — lee una tecla (independiente del sistema)
 *        c. processInput(key) — navega o ejecuta acciones
 *   5. Muestra el cursor (ScreenBuffer::showCursor())
 *
 * El bucle es síncrono y bloqueante. Solo termina cuando el usuario
 * presiona Q (state = CLOSED). Es llamado desde:
 *   - BattleSystem::doPlayerAction() (case 2: Inventario) en combate
 *   - GameManager::mostrarInventario() en el overworld
 */
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

        char key = Platform::getKey();
        processInput(key);
    }

    ScreenBuffer::showCursor();
}