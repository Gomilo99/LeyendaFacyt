#ifndef BATALLA_HPP
#define BATALLA_HPP

#include <map>
#include <memory>
#include <string>
#include "json.hpp"
#include "Objeto.hpp"
#include "Enemigo.hpp"
#include "Jugador.hpp"

using json = nlohmann::json;

inline std::string dataPath(const std::string& filename) {
    return std::string("json/") + filename;
}

inline std::string cachePath(const std::string& filename) {
    return std::string("cache/") + filename;
}

void limpiarBuffer();
void limpiarPantalla();

std::map<std::string, std::shared_ptr<Objeto>> cargarObjetosDesdeJSON(const std::string& archivo);
std::map<std::string, std::shared_ptr<Enemigo>> cargarEnemigosDesdeJSON(
    const std::string& archivo,
    const std::map<std::string, std::shared_ptr<Objeto>>& objetosDisponibles);
Jugador cargarHeroe(const std::string& archivo);
void guardarHeroe(const Jugador& jugador);
std::shared_ptr<Enemigo> generarEnemigoPorNivel(
    const std::map<std::string, std::shared_ptr<Enemigo>>& enemigos, int nivelMaxPermitido);
void batalla(Jugador& jugador, const std::map<std::string, std::shared_ptr<Enemigo>>& enemigos);

#endif
