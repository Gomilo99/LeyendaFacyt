#include "../lib/ArtLoader.hpp"

std::vector<std::string> ArtLoader::cargarArte(const std::string &ruta){
    std::vector<std::string> lineas;
    std::ifstream file(ruta);
    std::string linea;
    while (std::getline(file, linea)){
        lineas.push_back(linea);
    }
    return lineas;
}

void ArtLoader::dibujarArteCentrado(ScreenBuffer &buf, const std::vector<std::string> &arte, int color){
    int alto = (int)arte.size();
    for (int y = 0; y < alto; y++){
        int ancho = (int)arte[y].size();
        int xOff = (SCREEN_WIDTH - ancho) / 2;
        if (xOff < 0) xOff = 0;
        buf.drawString(xOff, y + 1, arte[y], color);
    }
}