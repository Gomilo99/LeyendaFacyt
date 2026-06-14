#include "../lib/cacheManager.hpp"
#include "../lib/config.hpp"
#include <fstream>
#include <iostream>
#include <filesystem> // C++17, o usa <windows.h> | CreateDirectory

namespace fs = std::filesystem;

bool CacheManager::existePartida(){
    return fs::exists(Config::SAVE_DIR + "partida.flag");
}

void CacheManager::crearPartida(const Mapa &mapa, const Jugador &Jugador){
    fs::create_directory(Config::SAVE_DIR);
    guardarMapa(mapa);
    guardarHeroe(Jugador);

    // Archivo flag que indica "hay partida guardada"
    std::ofstream flag(Config::SAVE_DIR + "partida.flag");
}

bool CacheManager::guardarMapa(const Mapa &mapa){
    return mapa.guardar(Config::mapaCache());
}