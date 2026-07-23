#include "../lib/DataManager.hpp"
#include "../lib/Config.hpp"
#include "../lib/json.hpp"
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <vector>

using json = nlohmann::json;

std::mt19937& DataManager::rng() {
    static std::mt19937 engine(std::random_device{}());
    return engine;
}

std::map<std::string, std::shared_ptr<Objeto>> DataManager::cargarObjetos() {
    std::map<std::string, std::shared_ptr<Objeto>> objetos;
    std::ifstream file(Config::objetosPath());
    if (!file.is_open()) return objetos;
    json j;
    file >> j;

    if (j.contains("arma")) {
        for (const auto& item : j["arma"]) {
            std::string nombre = item["nombre"];
            std::string descripcion = item["descripcion"];
            int dano = item["dano"];
            objetos[nombre] = std::make_shared<Arma>(nombre, descripcion, dano);
        }
    }
    if (j.contains("pocion")) {
        for (const auto& item : j["pocion"]) {
            std::string nombre = item["nombre"];
            std::string descripcion = item["descripcion"];
            int curacion = item["curacion"];
            objetos[nombre] = std::make_shared<Pocion>(nombre, descripcion, curacion);
        }
    }
    if (j.contains("clave")) {
        for (const auto& item : j["clave"]) {
            std::string nombre = item["nombre"];
            std::string descripcion = item["descripcion"];
            objetos[nombre] = std::make_shared<ObjClave>(nombre, descripcion);
        }
    }
    file.close();
    return objetos;
}