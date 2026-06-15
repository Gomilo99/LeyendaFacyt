#ifndef DATA_MANAGER_HPP
#define DATA_MANAGER_HPP

#include <map>
#include <memory>
#include <string>
#include <random>
#include "objeto.hpp"
#include "enemigo.hpp"
#include "jugador.hpp"

namespace DataManager {
    std::mt19937& rng();

    std::map<std::string, std::shared_ptr<Objeto>> cargarObjetos();
    std::map<std::string, std::shared_ptr<Enemigo>> cargarEnemigos(
        const std::map<std::string, std::shared_ptr<Objeto>>& objetosDisponibles);

    Jugador cargarHeroe();
    void guardarHeroe(const Jugador& jugador);

    std::shared_ptr<Enemigo> generarEnemigoPorNivel(
        const std::map<std::string, std::shared_ptr<Enemigo>>& enemigos, int nivelMaxPermitido);
}

#endif
