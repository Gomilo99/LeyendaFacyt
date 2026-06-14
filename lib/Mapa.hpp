#ifndef MAPA_HPP
#define MAPA_HPP

#include <string>
#include <vector>

class Mapa{
    private:
        std::vector<std::string> grid;
        int ancho, alto;
    public:
        Mapa() : ancho(0), alto(0) {}
        bool cargar(const std::string &archivo);
        char getTile(int x, int y) const;
        void setTile(int x, int y, char c);
        int getAncho() const { return ancho; }
        int getAlto() const { return alto; }

        bool esTransitable(int x, int y) const;

        bool guardar(const std::string &archivo) const;
};

#endif