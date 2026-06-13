#ifndef MAPA_HPP
#define MAPA_HPP

#include <string>
#include <vector>

class Mapa {
private:
    std::vector<std::string> grid;
    int ancho, alto;
public:
    Mapa() : ancho(0), alto(0) {}
    bool cargar(const std::string& archivo);
    bool cargarConCache(const std::string& archivoOriginal, const std::string& archivoCache);
    bool guardarCache(const std::string& archivoCache) const;
    char getTile(int x, int y) const;
    bool esTransitable(int x, int y) const;
    void setTile(int x, int y, char c);
    int getAncho() const { return ancho; }
    int getAlto() const { return alto; }
};

#endif