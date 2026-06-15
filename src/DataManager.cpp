#include "../lib/DataManager.hpp"
#include "../lib/config.hpp"
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

Jugador DataManager::cargarHeroe() {
    std::ifstream file(Config::heroePath());
    if (!file.is_open()) return Jugador("Heroe");
    json j;
    file >> j;
    std::string nombre = j["nombre"];
    int salud = j["salud"];
    int ataque = j["ataque"];
    int defensa = j["defensa"];
    int nivel = j["nivel"];
    int pociones = j.contains("pociones") ? (int)j["pociones"] : 3;
    Jugador jugador(nombre, salud, ataque, defensa, nivel, pociones);
    file.close();
    return jugador;
}

void DataManager::guardarHeroe(const Jugador& jugador) {
    json j;
    j["nombre"] = jugador.getNombre();
    j["salud"] = jugador.getSalud();
    j["ataque"] = jugador.getAtaque();
    j["defensa"] = jugador.getDefensa();
    j["nivel"] = jugador.getNivel();
    j["pociones"] = jugador.getPociones();
    j["mana"] = jugador.getMana();
    std::ofstream file(Config::heroePath());
    file << j.dump(4);
    file.close();
}
