#ifndef ART_LOADER_HPP
#define ART_LOADER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "Batalla.hpp"

namespace ArtLoader {
    std::vector<std::string> cargarArte(const std::string &ruta);
    void dibujarArteCentrado(ScreenBuffer &buf, const std::vector<std::string> &arte, int color);
}

#endif