#include "../lib/Jugador.hpp"
#include "../lib/GameBalance.hpp"
#include <iostream>
#include <algorithm>

Jugador::Jugador(std::string nombre)
    : Personaje(nombre, STAT_BASE_SALUD, STAT_BASE_ATAQUE, STAT_BASE_DEFENSA, 1), 
    pociones(STAT_BASE_POCIONES), mana(STAT_BASE_MANA), manaMaxima(STAT_BASE_MANA),
    armaEquipada(nullptr), experiencia(0), posX(1), posY(1) {}

Jugador::Jugador(std::string nom, int hp, int atk, int def, int lvl, int poc)
    : Personaje(nom, hp, atk, def, lvl), pociones(poc), 
    mana(STAT_BASE_MANA + lvl * 10), manaMaxima(STAT_BASE_MANA + lvl * 10),
    armaEquipada(nullptr), experiencia(0) {}

void Jugador::atacar(Personaje* objetivo) {
    std::cout << nombre << " Atacas a " << objetivo->getNombre() << "!\n";
    objetivo->recibirDano(ataque);
}

void Jugador::usarPocion() {
    if(pociones > 0){
        int curacion = POCION_CURACION_DEFAULT;
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
    if (mana >= COSTO_MAGIA) {
        int danoMagico = ataque * MULT_DANO_MAGICO + nivel * BONUS_DANO_NIVEL;
        mana -= COSTO_MAGIA;
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

void Jugador::agregarObjetoSilencioso(std::shared_ptr<Objeto> objeto){
    std::string nombre = objeto->getNombre();
    inventario[nombre]++;
    objetosInventario[nombre] = objeto;
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

void Jugador::equiparArma(std::shared_ptr<Arma> nuevaArma, bool silencioso){
    if(armaEquipada){
        ataque -= armaEquipada->getDano();
    }
    armaEquipada = nuevaArma;
    ataque += nuevaArma->getDano();
    if (!silencioso)
        std::cout << "Has equipado el arma: " << nuevaArma->getNombre() << "\n";
}

std::vector<std::pair<std::string, std::shared_ptr<Objeto> >> Jugador::getItemsList() const {
    std::vector<std::pair<std::string, std::shared_ptr<Objeto> >> items;
    for (const auto &par : objetosInventario){
        items.push_back(par);
    }
    return items;
}

void Jugador::obtenerExperiencia(int cantidad) {
    if (!ignorarLimiteNivel && nivel >= nivelMaximoPermitido){
        experiencia = expNecesaria;
        return;
    }

    experiencia = std::min(expNecesaria, experiencia + std::max(0, cantidad));
    std::cout << "Has ganado " << cantidad << " de experiencia!\n";

    if(experiencia >= expNecesaria && (ignorarLimiteNivel || nivel < nivelMaximoPermitido)){
        std::cout << "Has subido de nivel!\n";
        saludMaxima += SALUD_POR_NIVEL * (nivel + 1);
        salud = saludMaxima;
        ataque += ATAQUE_POR_NIVEL * (nivel + 1);
        defensa += DEFENSA_POR_NIVEL * (nivel + 1);
        nivel++;
        expNecesaria += EXP_INCREMENTO;
        if(nivel == 3) expNecesaria = EXP_NIVEL_3;
        experiencia = std::min(experiencia, expNecesaria);

        std::cout << "Subida de Estadisticas!!\n";
        std::cout << "Nivel: " << nivel << " | Salud: " << salud << "/" << saludMaxima
                  << " | Ataque: " << ataque << " | Defensa: " << defensa
                  << " | Experiencia: " << experiencia << "/" << expNecesaria << std::endl;
    }
}
