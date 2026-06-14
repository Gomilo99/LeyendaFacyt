#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <optional>
#include "json.hpp"

struct Config{
    static const std::string DATA_DIR;
    static const std::string MAPS_DIR;
    static const std::string SAVE_DIR;

    static std::string objetosPath() { return DATA_DIR + "objetos.json"; }
    static std::string enemigosPath() { return DATA_DIR + "enemigos.json"; }
    static std::string heroePath()   { return SAVE_DIR + "heroe.json"; }
    static std::string mapaPath(int nivel = 1) { return MAPS_DIR + "nivel" + std::to_string(nivel) + ".txt"; }
    static std::string mapaCache(){ return SAVE_DIR + "mapa_cache.txt"; }
};
#endif