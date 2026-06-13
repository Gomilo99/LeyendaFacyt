#include "../lib/Mapa.hpp"
#include <fstream>
#include <iostream>

bool Mapa::cargar(const std::string &archivo){
    std::ifstream file(archivo);
    if(!file.is_open()){
        std::cerr << "No se pudo cargar el mapa: " << archivo << "\n";
        return false;
    }

    grid.clear();
    std::string linea;
    while(std::getline(file, linea)){
        if(!linea.empty()){
            grid.push_back(linea);
        }
    }

    alto = grid.size();
    ancho = alto > 0 ? grid[0].size() : 0;

    return true;
}

bool Mapa::cargarConCache(const std::string& archivoOriginal, const std::string& archivoCache) {
    std::ifstream cacheFile(archivoCache);
    if (cacheFile.is_open()) {
        cacheFile.close();
        return cargar(archivoCache);
    }
    if (!cargar(archivoOriginal)) {
        return false;
    }
    guardarCache(archivoCache);
    return true;
}

bool Mapa::guardarCache(const std::string& archivoCache) const {
    std::ofstream file(archivoCache);
    if (!file.is_open()) {
        std::cerr << "No se pudo guardar cache: " << archivoCache << "\n";
        return false;
    }
    for (const auto& linea : grid) {
        file << linea << "\n";
    }
    return true;
}

char Mapa::getTile(int x, int y) const{
    if(y < 0 || y >= alto || x < 0 || x >= ancho)
        return '#'; // fuera del mapa es pared
    
    return grid[y][x];
}
void Mapa::setTile(int x, int y, char c){
    if (y >= 0 && y < alto && x >=0 && x < ancho) grid[y][x] = c;
}
bool Mapa::esTransitable(int x, int y) const {
    char tile = getTile(x, y);
    return tile == '.' || tile == 'p' || tile == 'E' || tile == 'K' || tile == 'H';
}

