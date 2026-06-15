#include "../lib/cacheManager.hpp"
#include "../lib/config.hpp"
#include "../lib/batalla.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

bool CacheManager::existePartida(){
    return fs::exists(Config::SAVE_DIR + "partida.flag");
}

void CacheManager::crearPartida(const Mapa &mapa, const Jugador &jugador){
    fs::create_directory(Config::SAVE_DIR);
    guardarMapa(mapa);
    guardarHeroe(jugador);

    std::ofstream flag(Config::SAVE_DIR + "partida.flag");
}

bool CacheManager::guardarMapa(const Mapa &mapa){
    return mapa.guardar(Config::mapaCache());
}

bool CacheManager::cargarMapa(Mapa &mapa){
    return mapa.cargar(Config::mapaCache());
}

void CacheManager::guardarHeroe(const Jugador &jugador){
    ::guardarHeroe(jugador, Config::heroePath());
}

Jugador CacheManager::cargarHeroe(){
    return ::cargarHeroe(Config::heroePath());
}

void CacheManager::limpiar(){
    if (fs::exists(Config::SAVE_DIR)) {
        fs::remove_all(Config::SAVE_DIR);
    }
}