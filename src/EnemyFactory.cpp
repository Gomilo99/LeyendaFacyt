#include "../lib/EnemyFactory.hpp"
#include "../lib/DataManager.hpp"
#include "../lib/config.hpp"
#include "../lib/json.hpp"
#include <fstream>
#include <random>
#include <stdexcept>

using json = nlohmann::json;

void EnemyFactory::cargarDesdeJSON(
    const std::string& path,
    const std::map<std::string, std::shared_ptr<Objeto>>& objetos)
{
    std::ifstream file(path);
    if (!file.is_open()) return;

    json j;
    file >> j;

    for (auto it = j.begin(); it != j.end(); ++it) {
        int nivel = std::stoi(it.key());
        for (const auto& item : it.value()) {
            EnemyTemplate t;
            t.id = item["id"];
            t.nombre = item["nombre"];
            t.salud = item["salud"];
            t.ataque = item["ataque"];
            t.defensa = item["defensa"];
            t.peso = item.value("peso", 5);
            t.nivel = nivel;
            t.boss = item.value("boss", false);

            for (int i = 0; i < 6 && i < (int)item["ascii"].size(); i++)
                t.asciiArt[i] = item["ascii"][i];

            for (const auto& drop : item["botin"]) {
                std::string nombreObj = drop["nombre"];
                int prob = drop["prob"];
                auto itObj = objetos.find(nombreObj);
                if (itObj == objetos.end())
                    throw std::runtime_error("Objeto no encontrado: " + nombreObj);
                t.botin.emplace_back(itObj->second, prob);
            }

            plantillas[nivel].push_back(t);
        }
    }
}

int EnemyFactory::totalPeso(int nivel) const {
    auto it = plantillas.find(nivel);
    if (it == plantillas.end()) return 0;
    int suma = 0;
    for (const auto& t : it->second) suma += t.peso;
    return suma;
}

const EnemyFactory::EnemyTemplate& EnemyFactory::seleccionarPlantilla(int nivel) {
    auto it = plantillas.find(nivel);
    if (it == plantillas.end() || it->second.empty())
        throw std::runtime_error("No hay enemigos para el nivel " + std::to_string(nivel));

    int total = totalPeso(nivel);
    std::uniform_int_distribution<int> dist(0, total - 1);
    int tirada = dist(DataManager::rng());

    for (const auto& t : it->second) {
        tirada -= t.peso;
        if (tirada < 0) return t;
    }
    return it->second.back();
}

Enemigo EnemyFactory::crearEnemigo(int nivel) {
    const auto& t = seleccionarPlantilla(nivel);
    return Enemigo(t.id, t.nombre, t.salud, t.ataque, t.defensa, t.nivel, t.asciiArt, t.botin);
}

Enemigo EnemyFactory::crearJefe(int nivel) {
    for (int n = nivel; n >= 1; n--) {
        auto it = plantillas.find(n);
        if (it == plantillas.end()) continue;
        for (const auto& t : it->second) {
            if (t.boss)
                return Enemigo(t.id, t.nombre, t.salud, t.ataque, t.defensa, t.nivel, t.asciiArt, t.botin);
        }
    }
    throw std::runtime_error("No hay jefe definido para el nivel " + std::to_string(nivel));
}

bool EnemyFactory::hayJefe(int nivel) const {
    for (int n = nivel; n >= 1; n--) {
        auto it = plantillas.find(n);
        if (it == plantillas.end()) continue;
        for (const auto& t : it->second)
            if (t.boss) return true;
    }
    return false;
}
