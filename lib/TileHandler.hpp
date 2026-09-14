#ifndef TILE_HANDLER_HPP
#define TILE_HANDLER_HPP

#include <functional>
#include <map>

class GameManager;

/**
 * Registrador y despachador de eventos de tile.
 * Permite registrar manejadores para tipos de tile específicos (ej. 'B', 'K', 'H')
 * evitando condicionales hardcoded en GameManager.
 */
using TileAction = std::function<void(GameManager& gm, int x, int y)>;

class TileRegistry {
private:
    std::map<char, TileAction> handlers;

public:
    void registrarManejador(char tile, TileAction action) {
        handlers[tile] = action;
    }

    bool ejecutarManejador(char tile, GameManager& gm, int x, int y) {
        auto it = handlers.find(tile);
        if (it != handlers.end()) {
            it->second(gm, x, y);
            return true;
        }
        return false;
    }
};

#endif
