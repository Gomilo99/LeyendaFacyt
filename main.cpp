#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include "Batalla.hpp"

int main() {
    std::ifstream objetosFile("objetos.json");
    std::ifstream enemigosFile("enemigos.json");
    std::ifstream heroeFile("heroe.json");
    if (!objetosFile.is_open() && !enemigosFile.is_open()) {
        std::cerr << "Faltan archivos para ejecutar el juego.\n";
        objetosFile.close();
        enemigosFile.close();
        heroeFile.close();
        return 1;
    }
    objetosFile.close();
    enemigosFile.close();

    auto objetos = cargarObjetosDesdeJSON("objetos.json");
    if (objetos.empty()) {
        std::cerr << "No se pudieron cargar los objetos desde el archivo JSON.\n";
        heroeFile.close();
        return 1;
    }
    auto enemigos = cargarEnemigosDesdeJSON("enemigos.json", objetos);
    if (enemigos.empty()) {
        std::cerr << "No se pudieron cargar los enemigos desde el archivo JSON.\n";
        heroeFile.close();
        return 1;
    }

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    Jugador jugador("Heroe");
    if(heroeFile.is_open()) {
        jugador = cargarHeroe("heroe.json");
    }

    auto itEspada = objetos.find("Espada Gallo");
    if (itEspada != objetos.end()) {
        jugador.equiparArma(std::dynamic_pointer_cast<Arma>(itEspada->second));
    } else {
        std::cout << "No se encontró la espada en el inventario.\n";
    }
    while (jugador.estaVivo()){
        batalla(jugador, enemigos);
    }

    if(!jugador.estaVivo()) {
        std::cout << "\n\nGAME OVER";
        return 0;
    }
    std::cout << "\nMuchas gracias por Jugar :)" << std::endl;
    heroeFile.close();
    return 0;
}
