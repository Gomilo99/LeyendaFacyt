#include "../lib/cargadorArchivos.hpp"
#include <fstream>
#include <iostream>

bool Cargador::archivoExiste(const std::string &ruta){
    std::ifstream f(ruta);
    return f.is_open();
}

std::optional<nlohmann::json> Cargador::cargarJSON(const std::string &ruta){
    std::ifstream file(ruta);
    if (!file.is_open()){
        std::cerr << "ERROR: no se pudo abrir " << ruta << std::endl;
        return std::nullopt;
    }
    nlohmann::json j;
    try{
        file >> j;
    }catch (const std::exception &e){
        std::cerr << "Error al parsear " << ruta << ": " << e.what() << std::endl;
        return std::nullopt;
    }
    return j;
}