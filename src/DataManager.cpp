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

std::map<std::string, std::shared_ptr<Enemigo>> DataManager::cargarEnemigos(
    const std::map<std::string, std::shared_ptr<Objeto>>& objetosDisponibles)
{
    std::map<std::string, std::shared_ptr<Enemigo>> enemigos;
    std::ifstream file(Config::enemigosPath());
    if (!file.is_open()) return enemigos;
    json j;
    file >> j;

    for (auto it = j.begin(); it != j.end(); ++it) {
        int nivel = std::stoi(it.key());
        for (const auto& item : it.value()) {
            std::string nombre = item["nombre"];
            int salud = item["salud"];
            int ataque = item["ataque"];
            int defensa = item["defensa"];
            std::string nombreLoot1 = item["loot1"];
            std::string nombreLoot2 = item["loot2"];
            int prob1 = item.contains("prob1") ? (int)item["prob1"] : 70;
            int prob2 = item.contains("prob2") ? (int)item["prob2"] : 30;

            auto it1 = objetosDisponibles.find(nombreLoot1);
            auto it2 = objetosDisponibles.find(nombreLoot2);
            if (it1 == objetosDisponibles.end() || it2 == objetosDisponibles.end()) {
                throw std::runtime_error("Objeto no encontrado: " + nombreLoot1 + " o " + nombreLoot2);
            }

            Drop drop1(it1->second, prob1);
            Drop drop2(it2->second, prob2);

            auto enemigo = std::make_shared<Enemigo>(nombre, salud, ataque, defensa, nivel, drop1, drop2);
            enemigos[nombre] = enemigo;
        }
    }
    file.close();
    return enemigos;
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

std::shared_ptr<Enemigo> DataManager::generarEnemigoPorNivel(
    const std::map<std::string, std::shared_ptr<Enemigo>>& enemigos, int nivelMaxPermitido)
{
    std::vector<std::shared_ptr<Enemigo>> candidatos;
    for (const auto& par : enemigos) {
        if (par.second->getNivel() == nivelMaxPermitido)
            candidatos.push_back(par.second);
    }
    if (candidatos.empty())
        throw std::runtime_error("No hay enemigos disponibles para el nivel solicitado");
    std::uniform_int_distribution<int> dist(0, candidatos.size() - 1);
    return candidatos[dist(rng())];
}
