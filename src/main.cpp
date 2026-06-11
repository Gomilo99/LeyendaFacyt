#include <iostream>
#include <fstream>
#include "../lib/Batalla.hpp"

int main() {
    std::ifstream objetosFile(dataPath("objetos.json"));
    std::ifstream enemigosFile(dataPath("enemigos.json"));
    std::ifstream heroeFile(dataPath("heroe.json"));
    if (!objetosFile.is_open() || !enemigosFile.is_open()) {
        std::cerr << "Faltan archivos para ejecutar el juego.\n";
        objetosFile.close();
        enemigosFile.close();
        heroeFile.close();
        return 1;
    }
    objetosFile.close();
    enemigosFile.close();

    auto objetos = cargarObjetosDesdeJSON(dataPath("objetos.json"));
    if (objetos.empty()) {
        std::cerr << "No se pudieron cargar los objetos desde el archivo JSON.\n";
        heroeFile.close();
        return 1;
    }
    auto enemigos = cargarEnemigosDesdeJSON(dataPath("enemigos.json"), objetos);
    if (enemigos.empty()) {
        std::cerr << "No se pudieron cargar los enemigos desde el archivo JSON.\n";
        heroeFile.close();
        return 1;
    }

    Jugador jugador("Heroe");
    if(heroeFile.is_open()) {
        jugador = cargarHeroe(dataPath("heroe.json"));
    }

    auto itEspada = objetos.find("Espada Gallo");
    if (itEspada != objetos.end()) {
        jugador.equiparArma(std::dynamic_pointer_cast<Arma>(itEspada->second));
    } else {
        std::cout << "No se encontró la espada en el inventario.\n";
    }
    while (jugador.estaVivo() && !jugador.getHaGanado()){
        batalla(jugador, enemigos);
    }

    if (jugador.getHaGanado()) {
        std::cout << "\nMuchas gracias por Jugar :)" << std::endl;
    } else {
        std::cout << "\n\nGAME OVER";
    }
    heroeFile.close();
    return 0;
}
