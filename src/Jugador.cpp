#include "../lib/jugador.hpp"
#include <iostream>
#include <algorithm>

Jugador::Jugador(std::string nombre)
    : Personaje(nombre, 100, 15, 10, 1), pociones(3), mana(50), manaMaxima(50),
      armaEquipada(nullptr), experiencia(0), posX(1), posY(1) {}

Jugador::Jugador(std::string nom, int hp, int atk, int def, int lvl, int poc)
    : Personaje(nom, hp, atk, def, lvl), pociones(poc), mana(50 + lvl * 10), manaMaxima(50 + lvl * 10),
      armaEquipada(nullptr), experiencia(0) {}

void Jugador::atacar(Personaje* objetivo) {
    std::cout << nombre << " Atacas a " << objetivo->getNombre() << "!\n";
    objetivo->recibirDano(ataque);
}

void Jugador::usarPocion() {
    if(pociones > 0){
        int curacion = 30;
        salud = std::min(salud + curacion, saludMaxima);
        pociones--;
        std::cout << "Usas una pocion. Salud recuperdad: +" << curacion << std::endl;
    } else {
        std::cout << "No tienes pociones restantes!\n";
    }
}

void Jugador::usarPocion(Objeto* pocion){
    auto pocionPtr = dynamic_cast<Pocion*>(pocion);
    if (pocionPtr) {
        int curacion = pocionPtr->getCuracion();
        salud = std::min(salud + curacion, saludMaxima);
        std::cout << "\nSalud recuperada: +" << curacion << std::endl;
    } else {
        std::cout << "\nEl objeto no es una pocion valida.\n";
    }
}

void Jugador::usarMagia(Personaje* objetivo) {
    int costo = 10;
    if (mana >= costo) {
        int danoMagico = ataque * 2 + nivel * 5;
        mana -= costo;
        std::cout << nombre << " lanza un hechizo a " << objetivo->getNombre() << "!\n";
        objetivo->recibirDano(danoMagico);
    } else {
        std::cout << "No tienes suficiente mana!\n";
    }
}

void Jugador::mostrarEstado() const {
    std::cout << "\n" << nombre << " - Salud: " << salud << "/" << saludMaxima
              << " | Mana: " << mana << "/" << manaMaxima
              << " | Ataque: " << ataque << " | Defensa: " << defensa;
    if (armaEquipada) {
        std::cout << "\nArma equipada: " << armaEquipada->getNombre()
                  << " | dano: " << armaEquipada->getDano();
    } else {
        std::cout << "\nArma equipada: ninguna";
    }
    std::cout << "\nNivel: " << nivel << " | Experiencia: " << experiencia
              << "/" << expNecesaria << std::endl;
}

void Jugador::mostrarInventario(){
    std::cout << "Inventario:\n";
    for (const auto& par : inventario) {
        std::cout << "- " << par.first << " x" << par.second << "\n"
                  << objetosInventario[par.first]->getDescripcion() << std::endl;
    }
    if(armaEquipada){
        std::cout << "\nArma equipada: " << armaEquipada->getNombre()
                  << " (" << armaEquipada->getDano() << " de dano)\n"
                  << armaEquipada->getDescripcion() << std::endl;
    } else {
        std::cout << "No tienes un arma equipada.\n";
    }
    std::string seleccion;
    std::cout << "Deseas usar un objeto? (s/n): ";
    std::cin >> seleccion;
    std::cout << std::endl;
    if(seleccion == "s" || seleccion == "S") {
        std::string nombreObjeto;
        std::cout << "\nIngresa el nombre del objeto: ";
        std::cin >> nombreObjeto;
        auto itObj = objetosInventario.find(nombreObjeto);
        if (itObj != objetosInventario.end()) {
            usarPocion(itObj->second.get());
            eliminarObjeto(nombreObjeto);
        } else {
            std::cout << "No tienes ese objeto en tu inventario.\n";
        }
    }
}

void Jugador::agregarObjeto(std::shared_ptr<Objeto> objeto){
    std::string nombre = objeto->getNombre();
    inventario[nombre]++;
    objetosInventario[nombre] = objeto;

    auto arma = std::dynamic_pointer_cast<Arma>(objeto);
    if (arma) {
        std::cout << "Has encontrado el arma: " << arma->getNombre() << " (" << arma->getDano() << " de daño).\n";
        std::cout << "¿Deseas equiparla? (s/n): ";
        char r;
        std::cin >> r;
        if(r == 's' || r == 'S') {
            equiparArma(arma);
        }
    }
}

void Jugador::eliminarObjeto(const std::string& nombre){
    auto it = inventario.find(nombre);
    if (it != inventario.end()) {
        it->second -= 1;
        if (it->second <= 0) {
            inventario.erase(it);
            objetosInventario.erase(nombre);
        }
    } else {
        std::cout << "No tienes ese objeto en tu inventario.\n";
    }
}

void Jugador::equiparArma(std::shared_ptr<Arma> nuevaArma){
    if(armaEquipada){
        ataque -= armaEquipada->getDano();
    }
    armaEquipada = nuevaArma;
    ataque += nuevaArma->getDano();
    std::cout << "Has equipado el arma: " << nuevaArma->getNombre() << "\n";
}

void Jugador::obtenerExperiencia(int cantidad) {
    experiencia += cantidad;
    std::cout << "Has ganado " << cantidad << " de experiencia!\n";

    if(experiencia >= expNecesaria){
        std::cout << "Has subido de nivel!\n";
        saludMaxima += 50 * (nivel + 1);
        salud = saludMaxima;
        defensa += 5 * (nivel + 1);
        ataque += 5 * (nivel + 1);
        nivel++;
        expNecesaria += 200;
        if(nivel == 3) expNecesaria = 700;

        std::cout << "Subida de Estadisticas!!\n";
        std::cout << "Nivel: " << nivel << " | Salud: " << salud << "/" << saludMaxima
                  << " | Ataque: " << ataque << " | Defensa: " << defensa
                  << " | Experiencia: " << experiencia << "/" << expNecesaria << std::endl;
    }
}
