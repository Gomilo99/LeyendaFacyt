#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <limits>
#include <map>
#include <memory>
#include <vector>
#include <random>
#include "../lib/Batalla.hpp"

std::mt19937& rng() {
    static std::mt19937 engine(std::random_device{}());
    return engine;
}

void limpiarBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void limpiarPantalla() {
    std::cout << "\033[2J\033[1;1H";
}

std::map<std::string, std::shared_ptr<Objeto>> cargarObjetosDesdeJSON(const std::string& archivo) {
    std::map<std::string, std::shared_ptr<Objeto>> objetos;
    std::ifstream file(archivo);
    if (!file.is_open()) {
        return objetos;
    }
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

std::map<std::string, std::shared_ptr<Enemigo>> cargarEnemigosDesdeJSON(
    const std::string& archivo,
    const std::map<std::string, std::shared_ptr<Objeto>>& objetosDisponibles)
{
    std::map<std::string, std::shared_ptr<Enemigo>> enemigos;
    std::ifstream file(archivo);
    if (!file.is_open()) {
        return enemigos;
    }
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

            auto it1 = objetosDisponibles.find(nombreLoot1);
            auto it2 = objetosDisponibles.find(nombreLoot2);
            if (it1 == objetosDisponibles.end() || it2 == objetosDisponibles.end()) {
                throw std::runtime_error("Objeto no encontrado: " + nombreLoot1 + " o " + nombreLoot2);
            }

            Drop drop1(it1->second, 70);
            Drop drop2(it2->second, 30);

            auto enemigo = std::make_shared<Enemigo>(nombre, salud, ataque, defensa, nivel, drop1, drop2);
            enemigos[nombre] = enemigo;
        }
    }
    file.close();
    return enemigos;
}

Jugador cargarHeroe(const std::string& archivo) {
    std::string cacheArchivo = cachePath("heroe.cache");
    std::ifstream file(cacheArchivo);
    bool desdeCache = file.is_open();
    if (!desdeCache) {
        file.open(archivo);
    }
    json j;
    file >> j;
    std::string nombre = j["nombre"];
    int salud = j["salud"];
    int ataque = j["ataque"];
    int defensa = j["defensa"];
    int nivel = j["nivel"];
    int pociones = j["pociones"];
    Jugador jugador(nombre, salud, ataque, defensa, nivel, pociones);
    if (desdeCache && j.contains("posX")) {
        jugador.setPos(j["posX"], j["posY"]);
    }
    file.close();
    return jugador;
}

void guardarHeroe(const Jugador& jugador) {
    json j;
    j["nombre"] = jugador.getNombre();
    j["salud"] = jugador.getSalud();
    j["ataque"] = jugador.getAtaque();
    j["defensa"] = jugador.getDefensa();
    j["nivel"] = jugador.getNivel();
    j["pociones"] = jugador.getPociones();
    j["posX"] = jugador.getPosX();
    j["posY"] = jugador.getPosY();
    std::ofstream file(cachePath("heroe.cache"));
    file << j.dump(4);
    file.close();
}

std::shared_ptr<Enemigo> generarEnemigoPorNivel(
    const std::map<std::string, std::shared_ptr<Enemigo>>& enemigos, int nivelMaxPermitido)
{
    std::vector<std::shared_ptr<Enemigo>> candidatos;

    for (const auto& par : enemigos) {
        if (par.second->getNivel() == nivelMaxPermitido) {
            candidatos.push_back(par.second);
        }
    }

    if (candidatos.empty()) {
        throw std::runtime_error("No hay enemigos disponibles para el nivel solicitado");
    }

    std::uniform_int_distribution<int> dist(0, candidatos.size() - 1);
    return candidatos[dist(rng())];
}

void batalla(Jugador& jugador, const std::map<std::string, std::shared_ptr<Enemigo>>& enemigos) {
    limpiarPantalla();

    auto enemigoSelec = generarEnemigoPorNivel(enemigos, jugador.getNivel());
    Enemigo enemigo(*enemigoSelec);
    bool jefefinal = false;

    if (enemigo.getNivel() == 4) {
        std::cout << "Estas en una oscura cueva, siente una presencia extraña..." << std::endl;
        std::cout << "Ha aparecido el jefe final!!!\nHa aparecido " << enemigo.getNombre() << std::endl;
        jefefinal = true;
    } else {
        std::cout << "Un " << enemigo.getNombre() << " ha aparecido!\n";
    }

    std::cout << "Comienza la batalla!\n";
    while (jugador.estaVivo() && enemigo.estaVivo()) {
        jugador.mostrarMenu();
        limpiarBuffer();
        int opcion;
        std::cin >> opcion;

        switch (opcion) {
            case 1:
                limpiarPantalla();
                jugador.atacar(&enemigo);
                break;
            case 2:
                limpiarPantalla();
                jugador.usarPocion();
                break;
            case 3:
                jugador.mostrarInventario();
                continue;
            case 4:
                jugador.mostrarEstado();
                continue;
            default:
                std::cout << "Opcion invalida!\n";
                continue;
        }

        if (enemigo.estaVivo()) {
            enemigo.atacar(&jugador);
        }
    }

    if (!jugador.estaVivo()) {
        std::cout << "Has sido derrotado por " << enemigo.getNombre() << "!\n";
        guardarHeroe(jugador);
        return;
    }

    std::cout << "\n\nHAS DERROTADO A '" << enemigo.getNombre() << "' !\n";
    if (jefefinal && !enemigo.estaVivo()) {
        std::cout << "Felicidades mano, has derrotado al jefe final!\n";
        std::cout << "...\n";
        std::cout << "Has ganado el juego!\n";
        jugador.setHaGanado(true);
    }

    int exp = jugador.getNivel() * 50;
    jugador.obtenerExperiencia(exp);

    std::uniform_int_distribution<int> distLoot(0, 99);
    int chance = distLoot(rng());
    std::shared_ptr<Objeto> lootGanado = nullptr;

    Drop loot1 = enemigo.getLoot1();
    Drop loot2 = enemigo.getLoot2();
    if (chance < loot1.probabilidad) {
        lootGanado = loot1.objeto;
    } else if (chance < loot1.probabilidad + loot2.probabilidad) {
        lootGanado = loot2.objeto;
    }
    if (lootGanado) {
        std::cout << "Has obtenido: " << lootGanado->getNombre() << "\n";
        jugador.agregarObjeto(lootGanado);
    }

    guardarHeroe(jugador);
}
