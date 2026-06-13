#include <iostream>
#include <fstream>
#include <direct.h>
#include "../lib/Batalla.hpp"
#include "../lib/Mapa.hpp"

void crearDirectorioCache() {
    if (_mkdir("cache") != 0) {
        // ya existe o no se pudo crear, no pasa nada
    }
}

int main() {
    crearDirectorioCache();

    std::string OBJETOS_PATH = dataPath("objetos.json");
    std::string ENEMIGOS_PATH = dataPath("enemigos.json");
    std::string HEROE_PATH = dataPath("heroe.json");
    std::string MAPA_ORIGINAL = "mapas/nivel1.txt";
    std::string MAPA_CACHE = cachePath("nivel1.cache");

    std::ifstream objetosFile(OBJETOS_PATH);
    std::ifstream enemigosFile(ENEMIGOS_PATH);
    if (!objetosFile.is_open() || !enemigosFile.is_open()) {
        std::cerr << "Faltan archivos para ejecutar el juego.\n";
        objetosFile.close();
        enemigosFile.close();
        return 1;
    }
    objetosFile.close();
    enemigosFile.close();

    auto objetos = cargarObjetosDesdeJSON(OBJETOS_PATH);
    if (objetos.empty()) {
        std::cerr << "No se pudieron cargar los objetos desde el archivo JSON.\n";
        return 1;
    }
    auto enemigos = cargarEnemigosDesdeJSON(ENEMIGOS_PATH, objetos);
    if (enemigos.empty()) {
        std::cerr << "No se pudieron cargar los enemigos desde el archivo JSON.\n";
        return 1;
    }

    Mapa mapa;
    if (!mapa.cargarConCache(MAPA_ORIGINAL, MAPA_CACHE)) {
        std::cerr << "No se pudo cargar el mapa.\n";
        return 1;
    }

    Jugador jugador("Heroe");
    std::ifstream heroeFile(HEROE_PATH);
    std::ifstream heroeCacheFile(cachePath("heroe.cache"));
    bool hayCacheHeroe = heroeCacheFile.is_open();
    heroeCacheFile.close();

    if (heroeFile.is_open()) {
        jugador = cargarHeroe(HEROE_PATH);
    }
    heroeFile.close();

    if (!hayCacheHeroe) {
        for (int y = 0; y < mapa.getAlto(); y++) {
            for (int x = 0; x < mapa.getAncho(); x++) {
                if (mapa.getTile(x, y) == 'P') {
                    jugador.setPos(x, y);
                    mapa.setTile(x, y, '.');
                }
            }
        }
        mapa.guardarCache(MAPA_CACHE);
    }

    auto itEspada = objetos.find("Espada Gallo");
    if (itEspada != objetos.end()) {
        jugador.equiparArma(std::dynamic_pointer_cast<Arma>(itEspada->second));
    } else {
        std::cout << "No se encontró la espada en el inventario.\n";
    }

    bool explorando = true;
    while (jugador.estaVivo() && !jugador.getHaGanado() && explorando) {
        limpiarPantalla();

        for (int y = 0; y < mapa.getAlto(); y++) {
            for (int x = 0; x < mapa.getAncho(); x++) {
                if (x == jugador.getPosX() && y == jugador.getPosY())
                    std::cout << '@';
                else
                    std::cout << mapa.getTile(x, y);
            }
            std::cout << std::endl;
        }

        jugador.mostrarEstado();

        std::cout << "\nWASD mover | I inventario | Q salir: ";
        char input;
        std::cin >> input;

        int dx = 0, dy = 0;
        switch (input) {
            case 'w': case 'W': dy = -1; break;
            case 's': case 'S': dy =  1; break;
            case 'a': case 'A': dx = -1; break;
            case 'd': case 'D': dx =  1; break;
            case 'i': case 'I': jugador.mostrarInventario(); continue;
            case 'q': case 'Q': explorando = false; continue;
            default: continue;
        }

        int nuevoX = jugador.getPosX() + dx;
        int nuevoY = jugador.getPosY() + dy;

        if (!mapa.esTransitable(nuevoX, nuevoY)) {
            continue;
        }

        jugador.setPos(nuevoX, nuevoY);
        char tile = mapa.getTile(nuevoX, nuevoY);

        if (tile == 'E') {
            batalla(jugador, enemigos);
            if (jugador.estaVivo()) {
                mapa.setTile(nuevoX, nuevoY, '.');
                mapa.guardarCache(MAPA_CACHE);
            } else {
                explorando = false;
            }
        } else if (tile == 'K') {
            std::cout << "\nHas encontrado la llave magica!\n";
            jugador.setHaGanado(true);
        } else if (tile == 'H') {
            mapa.setTile(nuevoX, nuevoY, '.');
            mapa.guardarCache(MAPA_CACHE);
            jugador.usarPocion();
        }
    }

    if (jugador.getHaGanado()) {
        std::cout << "\nMuchas gracias por Jugar :)" << std::endl;
    } else if (!jugador.estaVivo()) {
        std::cout << "\n\nGAME OVER";
    }
    return 0;
}
