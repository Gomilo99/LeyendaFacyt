#ifndef CACHE_MANAGER_HPP
#define CACHE_MANAGER_HPP

#include <string>
#include <map>
#include <memory>
#include "Mapa.hpp"
#include "Jugador.hpp"
#include "Objeto.hpp"

namespace CacheManager{
    bool existePartida();
    void crearPartida(const Mapa &mapa, const Jugador &jugador);

    // Mapa
    bool guardarMapa(const Mapa &mapa);
    bool cargarMapa(Mapa &mapa);

    // Heroe
    void guardarHeroe(const Jugador &jugador);
    Jugador cargarHeroe(const std::map<std::string, std::shared_ptr<Objeto>>& objetos);

    // Limpiar cache (para nueva partida)
    void limpiar();
}

#endif