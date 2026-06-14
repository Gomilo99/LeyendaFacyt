#ifndef CARGADOR_ARCHIVOS_HPP
#define CARGADOR_ARCHIVOS_HPP

#include <optional>
#include "json.hpp"

struct Cargador {
    static bool archivoExiste(const std::string &ruta);
    static std::optional<nlohmann::json> cargarJSON(const std::string &ruta);
};

#endif