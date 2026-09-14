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
    objetivo->recibirDano(ataque);
}

void Jugador::usarPocion() {
    if(pociones > 0){
        int curacion = POCION_CURACION_DEFAULT;
        salud = std::min(salud + curacion, saludMaxima);
        pociones--;
    }
}

void Jugador::usarPocion(Objeto* pocion){
    auto pocionPtr = dynamic_cast<Pocion*>(pocion);
    if (pocionPtr) {
        int curacion = pocionPtr->getCuracion();
        salud = std::min(salud + curacion, saludMaxima);
    }
}

void Jugador::usarMagia(Personaje* objetivo) {
    if (mana >= COSTO_MAGIA) {
        int danoMagico = ataque * MULT_DANO_MAGICO + nivel * BONUS_DANO_NIVEL;
        mana -= COSTO_MAGIA;
        objetivo->recibirDano(danoMagico);
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
            << "/" << getExperienciaNecesaria() << std::endl;
}

void Jugador::agregarObjeto(std::shared_ptr<Objeto> objeto){
    std::string nombre = objeto->getNombre();
    inventario[nombre]++;
    objetosInventario[nombre] = objeto;
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

LevelUpResult Jugador::obtenerExperiencia(int cantidad) {
    LevelUpResult res;
    res.expGanada = cantidad;
    res.nivelAnterior = this->nivel;

    if (!ignorarLimiteNivel && nivel >= nivelMaximoPermitido){
        experiencia = expRequerida(nivel);
        res.nivelNuevo = this->nivel;
        return res;
    }

    this->experiencia += std::max(0, cantidad);

    if (this->experiencia >= expRequerida(this->nivel)){
        res.subioDeNivel = true;
        // #14: incrementos FIJOS por nivel (curva lineal). Antes *(nivel+1) era cuadrático.
        res.saludMaxGanada = SALUD_POR_NIVEL;
        res.ataqueGanado = ATAQUE_POR_NIVEL;
        res.defensaGanada = DEFENSA_POR_NIVEL;

        this->saludMaxima += res.saludMaxGanada;
        this->salud = this->saludMaxima;
        this->ataque += res.ataqueGanado;
        this->defensa += res.defensaGanada;
        this->nivel++;
        res.nivelNuevo = this->nivel;

        // #15: la meta del nuevo nivel también sale de la tabla.
        this->experiencia = std::min(this->experiencia, expRequerida(this->nivel));
    }else{
        res.nivelNuevo = this->nivel;
    }
    return res;
}
