#include "../lib/CacheManager.hpp"
#include "../lib/Config.hpp"
#include "../lib/json.hpp"
#include "../lib/Jugador.hpp"
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
    if (!fs::exists(Config::mapaCache())) return false;
    return mapa.cargar(Config::mapaCache());
}

void CacheManager::guardarHeroe(const Jugador &jugador){
    json j;
    j["nombre"]     = jugador.getNombre();
    j["salud"]      = jugador.getSalud();
    j["saludMaxima"] = jugador.getSaludMaxima();
    j["ataque"]     = jugador.getAtaque();
    j["defensa"]    = jugador.getDefensa();
    j["nivel"]      = jugador.getNivel();
    j["pociones"]   = jugador.getPociones();
    j["mana"]       = jugador.getMana();
    j["manaMaxima"] = jugador.getManaMaxima();
    j["posX"]       = jugador.getPosX();
    j["posY"]       = jugador.getPosY();
    j["exp"]        = jugador.getExperiencia();
    j["expMax"]     = jugador.getExperienciaNecesaria();
    j["arma"]       = jugador.getArmaNombre();

    json inventarioArr = json::array();
    for (const auto& [nombre, cant] : jugador.getInventario())
        inventarioArr.push_back({{"nombre", nombre}, {"cant", cant}});
    j["inventario"] = inventarioArr;

    std::ofstream file(Config::heroeCachePath());
    file << j.dump(4);
}

Jugador CacheManager::cargarHeroe(const std::map<std::string, std::shared_ptr<Objeto>>& objetos){
    std::ifstream file(Config::heroeCachePath());
    if (!file.is_open()) return Jugador("Heroe");
    json j;
    file >> j;

    std::string nombre    = j["nombre"];
    int salud             = j["salud"];
    int ataque            = j["ataque"];
    int defensa           = j["defensa"];
    int nivel             = j["nivel"];
    int pociones          = j.value("pociones", 3);
    int mana              = j.value("mana", 50);
    int exp               = j.value("exp", 0);
    int expMax            = j.value("expMax", 100);
    int posX              = j.value("posX", 1);
    int posY              = j.value("posY", 1);

    Jugador jugador(nombre, salud, ataque, defensa, nivel, pociones);
    jugador.setPos(posX, posY);
    jugador.setExperiencia(exp);
    jugador.setExperienciaNecesaria(expMax);
    jugador.setMana(mana);

    // Cargar inventario desde el array
    if (j.contains("inventario")) {
        for (const auto& item : j["inventario"]) {
            std::string nombreObj = item["nombre"];
            int cant = item["cant"];
            auto it = objetos.find(nombreObj);
            if (it != objetos.end()) {
                for (int i = 0; i < cant; i++)
                    jugador.agregarObjetoSilencioso(it->second);
            }
        }
    }

    // Equipar arma guardada
    if (j.contains("arma") && !j["arma"].get<std::string>().empty()) {
        std::string armaNombre = j["arma"];
        auto it = objetos.find(armaNombre);
        if (it != objetos.end()) {
            auto arma = std::dynamic_pointer_cast<Arma>(it->second);
            if (arma)
                jugador.equiparArma(arma, true);
        }
    }

    file.close();
    return jugador;
}

void CacheManager::limpiar(){
    if (fs::exists(Config::SAVE_DIR))
        fs::remove_all(Config::SAVE_DIR);
}