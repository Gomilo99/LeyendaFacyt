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

    // Verifica si la grilla tiene al menos una fila.
    alto = grid.size(); // Accede a la primera fila
    ancho = alto > 0 ? grid[0].size() : 0; // Obtiene el tamaño.

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

bool Mapa::guardar(const std::string &archivo) const{
    std::ofstream file(archivo);
    if(!file.is_open()) return false;
    for (const auto& linea : grid){
        file << linea << std::endl;
    }
    return true;
}