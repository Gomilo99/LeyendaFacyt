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
    guardarPartida(mapa, jugador);
    std::ofstream flag(Config::SAVE_DIR + "partida.flag");
}

void CacheManager::guardarPartida(const Mapa &mapa, const Jugador &jugador){
    guardarMapa(mapa);
    guardarHeroe(jugador);
}

bool CacheManager::guardarMapa(const Mapa &mapa){
    return mapa.guardar(Config::mapaCache());
}

bool CacheManager::cargarMapa(Mapa &mapa){
    if (!fs::exists(Config::mapaCache())) return false;
    return mapa.cargar(Config::mapaCache());
}

bool CacheManager::guardarEstado(const EstadoPartida& estado){
    std::ofstream file(Config::partidaCachePath());
    if (!file.is_open()) return false;

    json j;
    j["nivelActual"] = estado.nivelActual;
    j["jefeDerrotado"] = estado.jefeDerrotado;
    j["haGanadoFinal"] = estado.haGanadoFinal;
    file << j.dump(4);
    return file.good();
}

bool CacheManager::cargarEstado(EstadoPartida& estado){
    std::ifstream file(Config::partidaCachePath());
    if (!file.is_open()) return false;

    json j;
    file >> j;
    estado.nivelActual = j.value("nivelActual", 1);
    estado.jefeDerrotado = j.value("jefeDerrotado", false);
    estado.haGanadoFinal = j.value("haGanadoFinal", false);
    return true;
}

void CacheManager::guardarHeroe(const Jugador &jugador){
    json j;
    j["version"]     = 1;
    j["nombre"]      = jugador.getNombre();
    j["salud"]       = jugador.getSalud();
    j["saludMaxima"] = jugador.getSaludMaxima();
    j["ataque"]      = jugador.getAtaque();
    j["defensa"]     = jugador.getDefensa();
    j["nivel"]       = jugador.getNivel();
    j["pociones"]    = jugador.getPociones();
    j["mana"]        = jugador.getMana();
    j["manaMaxima"]  = jugador.getManaMaxima();
    j["posX"]        = jugador.getPosX();
    j["posY"]        = jugador.getPosY();
    j["exp"]         = jugador.getExperiencia();
    j["expMax"]      = jugador.getExperienciaNecesaria();
    j["arma"]        = jugador.getArmaNombre();
    j["nivelActual"] = jugador.getNivelActual();

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

    int version           = j.value("version", 1);
    std::string nombre    = j.value("nombre", "Heroe");
    int salud             = j.value("salud", 100);
    int saludMaxima       = j.value("saludMaxima", salud);
    int ataque            = j.value("ataque", 15);
    int defensa           = j.value("defensa", 10);
    int nivel             = j.value("nivel", 1);
    int pociones          = j.value("pociones", 3);
    int mana              = j.value("mana", 50);
    int manaMaxima        = j.value("manaMaxima", 50);
    int exp               = j.value("exp", 0);
    int expMax            = j.value("expMax", 100);
    int posX              = j.value("posX", 1);
    int posY              = j.value("posY", 1);
    int nivelActual       = j.value("nivelActual", 1);

    Jugador jugador(nombre, salud, ataque, defensa, nivel, pociones);
    jugador.setSaludMaxima(saludMaxima);
    jugador.setSalud(salud);
    jugador.setMana(mana);
    jugador.setManaMaxima(manaMaxima);
    jugador.setPos(posX, posY);
    jugador.setExperiencia(exp);
    jugador.setExperienciaNecesaria(expMax);
    jugador.setNivelActual(nivelActual);

    // Cargar inventario desde el array
    if (j.contains("inventario") && j["inventario"].is_array()) {
        for (const auto& item : j["inventario"]) {
            std::string nombreObj = item.value("nombre", "");
            int cant = item.value("cant", 0);
            auto it = objetos.find(nombreObj);
            if (it != objetos.end()) {
                for (int i = 0; i < cant; i++)
                    jugador.agregarObjetoSilencioso(it->second);
            }
        }
    }

    // Equipar arma guardada
    if (j.contains("arma") && j["arma"].is_string() && !j["arma"].get<std::string>().empty()) {
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