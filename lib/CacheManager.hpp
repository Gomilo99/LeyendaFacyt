#ifndef CACHE_MANAGER_HPP
#define CACHE_MANAGER_HPP

#include <string>
#include "Mapa.hpp"
#include "Jugador.hpp"
namespace CacheManager{
    bool existePartida();
    void crearPartida(const Mapa &mapa, const Jugador &jugador);

    // Mapa
    bool guardarMapa(const Mapa &mapa);
    bool cargarMapa(Mapa &mapa); // Si falla, carga el original

    // Heroe
    void guardarHeroe(const Jugador &Jugador);
    Jugador cargarHeroe();

    // Limpiar cache (para nueva partida)
    void limpiar();
}
#endif