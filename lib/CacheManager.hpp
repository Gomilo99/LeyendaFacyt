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

    // Guardado completo unificado (Mapa + Heroe)
    void guardarPartida(const Mapa &mapa, const Jugador &jugador);

    // Guardado individual de componentes
    bool guardarMapa(const Mapa &mapa);
    bool cargarMapa(Mapa &mapa);

    void guardarHeroe(const Jugador &jugador);
    Jugador cargarHeroe(const std::map<std::string, std::shared_ptr<Objeto>>& objetos);

    // Limpiar cache (para nueva partida)
    void limpiar();
}

#endif