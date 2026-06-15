#ifndef DATA_MANAGER_HPP
#define DATA_MANAGER_HPP

#include <map>
#include <memory>
#include <string>
#include <random>
#include "objeto.hpp"
#include "jugador.hpp"

namespace DataManager {
    std::mt19937& rng();

    std::map<std::string, std::shared_ptr<Objeto>> cargarObjetos();

    Jugador cargarHeroe();
    void guardarHeroe(const Jugador& jugador);
}

#endif
