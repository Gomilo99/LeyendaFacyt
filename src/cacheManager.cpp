#include "../lib/cacheManager.hpp"
#include "../lib/config.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

using json = nlohmann::json;
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
    json j;
    j["nombre"] = jugador.getNombre();
    j["salud"] = jugador.getSalud();
    j["ataque"] = jugador.getAtaque();
    j["defensa"] = jugador.getDefensa();
    j["nivel"] = jugador.getNivel();
    j["pociones"] = jugador.getPociones();
    j["mana"] = jugador.getMana();
    std::ofstream file(Config::heroeCachePath());
    file << j.dump(4);
}

Jugador CacheManager::cargarHeroe(){
    std::ifstream file(Config::heroeCachePath());
    json j;
    file >> j;
    std::string nombre = j["nombre"];
    int salud = j["salud"];
    int ataque = j["ataque"];
    int defensa = j["defensa"];
    int nivel = j["nivel"];
    int pociones = j.contains("pociones") ? (int)j["pociones"] : 3;
    file.close();
    return Jugador(nombre, salud, ataque, defensa, nivel, pociones);
}

void CacheManager::limpiar(){
    if (fs::exists(Config::SAVE_DIR)) {
        fs::remove_all(Config::SAVE_DIR);
    }
}