#include "../lib/EnemyFactory.hpp"
#include "../lib/DataManager.hpp"
#include "../lib/config.hpp"
#include "../lib/json.hpp"
#include <fstream>
#include <random>
#include <stdexcept>

using json = nlohmann::json;

/**
 * Carga todas las plantillas desde el archivo JSON.
 *
 * Estructura esperada del JSON:
 * {
 *   "1": [ { "id": "...", "nombre": "...", ... }, ... ],
 *   "2": [ ... ],
 *   ...
 * }
 *
 * Cada enemigo debe tener "id", "nombre", "salud", "ataque", "defensa",
 * "peso", "ascii" (array de 6 strings), "botin" (array de {nombre, prob})
 * y "boss" (bool).
 *
 * Los nombres de objetos en "botin" se resuelven contra el map de objetos
 * cargados previamente. Si algún objeto no existe, se lanza excepción.
 */
void EnemyFactory::cargarDesdeJSON(
    const std::string& path,
    const std::map<std::string, std::shared_ptr<Objeto>>& objetos)
{
    std::ifstream file(path);
    if (!file.is_open()) return;

    json j;
    file >> j;

    // Itera cada nivel del JSON ("1", "2", "3", "4")
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

            // Copia las 6 líneas de arte ASCII
            for (int i = 0; i < 6 && i < (int)item["ascii"].size(); i++)
                t.asciiArt[i] = item["ascii"][i];

            // Resuelve cada objeto del botín contra el catálogo
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

/**
 * Suma todos los pesos de los enemigos de un nivel.
 * Se usa para determinar el rango del random en seleccionarPlantilla.
 */
int EnemyFactory::totalPeso(int nivel) const {
    auto it = plantillas.find(nivel);
    if (it == plantillas.end()) return 0;
    int suma = 0;
    for (const auto& t : it->second) suma += t.peso;
    return suma;
}

/**
 * Algoritmo de ruleta ponderada (weighted random selection).
 *
 * 1. Suma todos los pesos de los enemigos del nivel → totalPeso
 * 2. Tira uniform_int_distribution(0, totalPeso - 1) → tirada
 * 3. Itera los enemigos restando su peso de la tirada
 * 4. El primer enemigo que deja la tirada negativa es el seleccionado
 *
 * Esto da más probabilidad de aparición a los de mayor peso.
 */
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

/**
 * Crea una instancia de Enemigo a partir de una plantilla aleatoria
 * del nivel indicado. La copia incluye stats, arte ASCII y botín.
 */
Enemigo EnemyFactory::crearEnemigo(int nivel) {
    const auto& t = seleccionarPlantilla(nivel);
    return Enemigo(t.id, t.nombre, t.salud, t.ataque, t.defensa, t.nivel, t.asciiArt, t.botin);
}

/**
 * Busca el primer jefe (boss: true) desde el nivel actual hacia abajo.
 * Esto permite tener jefes solo en ciertos niveles (ej. nivel 4)
 * y que niveles inferiores busquen hacia arriba o lancen excepción.
 */
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

/**
 * Verifica si existe algún jefe entre el nivel 1 y el nivel dado.
 * Se usa en GameManager para decidir si mostrar el mensaje de jefe
 * o caer en un enemigo aleatorio normal.
 */
bool EnemyFactory::hayJefe(int nivel) const {
    for (int n = nivel; n >= 1; n--) {
        auto it = plantillas.find(n);
        if (it == plantillas.end()) continue;
        for (const auto& t : it->second)
            if (t.boss) return true;
    }
    return false;
}
