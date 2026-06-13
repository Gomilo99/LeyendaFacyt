#include <iostream>
#include <fstream>
#include "../lib/Batalla.hpp"
#include "../lib/Mapa.hpp"

int main() {
    std::string OBJETOS_DATAPATH = "json/objetos.json";
    std::string ENEMIGOS_DATAPATH = "json/enemigos.json";
    std::string HEROE_DATAPATH = "json/heroe.json";
    std::string MAPAS_DATAPATH = "../mapas/nivel.txt";

    // Carga de archivos JSON
    std::ifstream objetosFile(OBJETOS_DATAPATH);
    std::ifstream enemigosFile(ENEMIGOS_DATAPATH);
    std::ifstream heroeFile(HEROE_DATAPATH);

    if (!objetosFile.is_open() || !enemigosFile.is_open()) {
        std::cerr << "Faltan archivos para ejecutar el juego.\n";
        objetosFile.close();
        enemigosFile.close();
        heroeFile.close();
        return 1;
    }
    objetosFile.close();
    enemigosFile.close();

    // Una vez se verifica todo, se carga la información
    auto objetos = cargarObjetosDesdeJSON(OBJETOS_DATAPATH);
    if (objetos.empty()) {
        std::cerr << "No se pudieron cargar los objetos desde el archivo JSON.\n";
        heroeFile.close();
        return 1;
    }
    auto enemigos = cargarEnemigosDesdeJSON(ENEMIGOS_DATAPATH, objetos);
    if (enemigos.empty()) {
        std::cerr << "No se pudieron cargar los enemigos desde el archivo JSON.\n";
        heroeFile.close();
        return 1;
    }

    Mapa mapa;
    if(!mapa.cargar(MAPAS_DATAPATH)){
        std::cerr << "No se pudo cargar el mapa. ERROR: Archivo no encontrado. \n";
        return 1;
    }

    Jugador jugador("Heroe");
    if(heroeFile.is_open()) {
        jugador = cargarHeroe(HEROE_DATAPATH);
    }

    // Buscar tile 'P' para gestionar al jugar
    for(int y = 0; y < mapa.getAlto(); y++){
        for(int x = 0; x < mapa.getAncho(); x++){
            if(mapa.getTile(x, y) == 'P'){
                jugador.setPos(x, y);
            }
        }
    }

    // Cargar datos iniciales
    auto itEspada = objetos.find("Espada Gallo");
    if (itEspada != objetos.end()) {
        jugador.equiparArma(std::dynamic_pointer_cast<Arma>(itEspada->second));
    } else {
        std::cout << "No se encontró la espada en el inventario.\n";
    }

    // Bucle principal: EXPLORE -> COMBAT
    bool explorando = true;
    while (jugador.estaVivo() && !jugador.getHaGanado() && explorando){
        limpiarPantalla();

        // Renderizar mapa (top-down)
        for (int y = 0; y < mapa.getAlto(); y++){
            for (int x = 0; x < mapa.getAncho(); x++){
                if (x == jugador.getPosX() && y == jugador.getPosY()){
                    std::cout << '@';
                }else{
                    std::cout << mapa.getTile(x, y);
                }
                std::cout << std::endl;
            }
        }

        // Selección de acciones
        std::cout << "\nWASD para mover, I inventario, Q salir: ";
        char input;
        std::cin >> input;
        int dx = 0, dy = 0;
        switch (input)
        {
        case 'w': case 'W': dy = -1; break;
        case 's': case 'S': dy = 1; break;
        case 'a': case 'A': dx = -1; break;
        case 'd': case 'D': dx = 1; break;
        case 'i': case 'I': jugador.mostrarInventario(); continue;
        case 'q': case 'Q': explorando = false; continue;
        default: continue;
        }

        // Detección de nuevo valores de movimiento
        int nuevoX = jugador.getPosX() + dx;
        int nuevoY = jugador.getPosY() + dy;

        if (mapa.esTransitable(nuevoX, nuevoY)){
            char tile = mapa.getTile(nuevoX, nuevoY);
            if (tile == 'E'){
                // Transición de combate
                batalla(jugador, enemigos);
                // Marcar tile 'E' como '.' para que no reaparezca
                // (rquiere modificar el mapa: grid mutable)
                if (jugador.estaVivo()){
                    // Limpiar la E del mapa
                    // (necesitas un setTile en Mapa)
                }
            }
            if (tile == 'K'){
                std::cout << "Has encontrado la llave mágica!\n";
                jugador.setHaGanado(true);
            }
            if (tile == 'H'){
                jugador.usarPocion();
                // Marcar H como .
            }
        }
    }

    // Cerrar el juego
    if (jugador.getHaGanado()) {
        std::cout << "\nMuchas gracias por Jugar :)" << std::endl;
    } else {
        std::cout << "\n\nGAME OVER";
    }
    heroeFile.close();
    return 0;
}
