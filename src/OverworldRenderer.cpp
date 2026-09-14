#include "../lib/OverworldRenderer.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace {
    // Ancho base del recuadro HUD; crece según la línea más larga.
    const int HUD_ANCHO_BASE = 40;
    // Separación en columnas entre el mapa y el HUD.
    const int HUD_SEPARACION = 2;
}

int OverworldRenderer::colorDeTile(char t, const MapMetadata& metadata) const {
    // Zonas de terreno del .meta tienen prioridad sobre el tipo visual.
    const ZoneMetadata* zone = metadata.zoneAt(t);
    const bool esTerreno = t == '.' || t == ',' || t == ';' ||
                           t == '~' || t == 'd' || t == 'f' ||
                           t == '^' || t == 's';
    if (zone && esTerreno)
        return zone->terrainColor;

    switch (t) {
        case '#': case '-': case '|': case '+': case '=':
            return COL_CYAN;
        case '.': case 'd':
            return 90;               // gris claro (bright black ANSI)
        case ',': case ';':
            return COL_YELLOW;
        case '~':
            return COL_BLUE;
        case 's': case 'H': case 'G':
            return COL_BGREEN;
        case 'P':
            return COL_BYELLOW;
        case 'K':
            return COL_BCYAN;
        case 'B':
            return COL_BRED;
        case 'h':
            return COL_GREEN;
        default:
            return COL_DEFAULT;
    }
}

void OverworldRenderer::dibujarMapa(const Jugador& jugador, const Mapa& mapa,
                                    const MapMetadata& metadata) {
    for (int y = 0; y < mapa.getAlto(); y++) {
        for (int x = 0; x < mapa.getAncho(); x++) {
            if (x == jugador.getPosX() && y == jugador.getPosY()) {
                buf.setChar(x, y, '@', COL_BYELLOW);
            } else {
                const char t = mapa.getTile(x, y);
                buf.setChar(x, y, t, colorDeTile(t, metadata));
            }
        }
    }
}

void OverworldRenderer::dibujarHud(const Jugador& jugador, int anchoMapa,
                                   const MapMetadata& metadata,
                                   const ZoneMetadata* zona, int nivelMaximoSeccion,
                                   bool limiteNivelActivo) {
    const int barW = 10;
    const int hpFill = (jugador.getSaludMaxima() > 0)
        ? (jugador.getSalud() * barW / jugador.getSaludMaxima()) : 0;
    const int mpFill = (jugador.getManaMaxima() > 0)
        ? (jugador.getMana() * barW / jugador.getManaMaxima()) : 0;
    const std::string hpBar = std::string(hpFill, '#') + std::string(barW - hpFill, '.');
    const std::string mpBar = std::string(mpFill, '#') + std::string(barW - mpFill, '.');

    const float hpPct = (jugador.getSaludMaxima() > 0)
        ? (jugador.getSalud() * 100.0f / jugador.getSaludMaxima()) : 0.0f;
    const int hpColor = (hpPct > 50.0f) ? COL_GREEN :
                        (hpPct > 25.0f) ? COL_YELLOW : COL_RED;

    std::vector<std::string> lineas;
    lineas.push_back(jugador.getNombre());
    lineas.push_back("Nv: " + std::to_string(jugador.getNivel())
        + "  Exp: " + std::to_string(jugador.getExperiencia())
        + "/" + std::to_string(jugador.getExperienciaNecesaria()));
    lineas.push_back("HP: " + std::to_string(jugador.getSalud())
        + "/" + std::to_string(jugador.getSaludMaxima()) + " " + hpBar);
    lineas.push_back("MP: " + std::to_string(jugador.getMana())
        + "/" + std::to_string(jugador.getManaMaxima()) + " " + mpBar);
    lineas.push_back("Arma: " + jugador.getArmaNombre());
    lineas.push_back("Pociones: " + std::to_string(jugador.getPociones()));
    lineas.push_back("Sec: " + std::to_string(metadata.section)
        + "  Terreno: " + (zona ? zona->terrain : "default")
        + "  Zona: " + (zona ? zona->id : "none"));
    lineas.push_back("Cap: " + std::to_string(nivelMaximoSeccion)
        + (limiteNivelActivo ? " (ON)" : " (OFF)") + "  F8/8 toggle");

    int hudWidth = HUD_ANCHO_BASE;
    for (const auto& linea : lineas)
        hudWidth = std::max(hudWidth, static_cast<int>(linea.size()) + 4);

    // El HUD nunca se sale de la pantalla aunque el mapa sea muy ancho.
    const int hudX = std::min(anchoMapa + HUD_SEPARACION,
                              SCREEN_WIDTH - hudWidth - 1);

    buf.drawBox(hudX, 0, hudWidth, static_cast<int>(lineas.size()) + 2, COL_CYAN);
    for (size_t i = 0; i < lineas.size(); i++) {
        int color = COL_BWHITE;
        if (i == 0)         color = COL_BYELLOW;
        else if (i == 2)    color = hpColor;
        else if (i == 3)    color = COL_BBLUE;

        std::string contenido = "  " + lineas[i];
        contenido.resize(hudWidth - 2, ' ');
        buf.drawString(hudX + 1, 1 + static_cast<int>(i), contenido, color);
    }
}

void OverworldRenderer::renderAll(const Jugador& jugador, const Mapa& mapa,
                                  const MapMetadata& metadata, const ZoneMetadata* zona,
                                  int nivelMaximoSeccion, bool limiteNivelActivo) {
    buf.forceRedraw();
    buf.clear();
    dibujarMapa(jugador, mapa, metadata);
    dibujarHud(jugador, mapa.getAncho(), metadata, zona,
               nivelMaximoSeccion, limiteNivelActivo);
    buf.render();
}