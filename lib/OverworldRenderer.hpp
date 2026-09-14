#ifndef OVERWORLD_RENDERER_HPP
#define OVERWORLD_RENDERER_HPP

/**
 * @file OverworldRenderer.hpp
 * Renderizador del mundo (mapa top-down + HUD del jugador).
 *
 * Extraído de GameManager::renderMapa (auditoría #18): el motor ahora solo
 * provee los datos (jugador, mapa, metadatos de zona) y este renderer se
 * encarga de pintarlos sobre un ScreenBuffer, como ya hacen batalla e
 * inventario. GameManager decide; el renderer dibuja.
 */

#include "Batalla.hpp"
#include "Mapa.hpp"
#include "Jugador.hpp"
#include "MapMetadata.hpp"

class OverworldRenderer {
    ScreenBuffer buf;

    // Devuelve el color ANSI del tile: primero las zonas de terreno definidas
    // en el .meta y después los tipos visuales clásicos del mapa.
    int colorDeTile(char t, const MapMetadata& metadata) const;
    // Pinta el grid del mapa 1:1. El '@' del jugador sobrescribe su tile.
    void dibujarMapa(const Jugador& jugador, const Mapa& mapa, const MapMetadata& metadata);
    // Pinta el recuadro HUD (nombre, nivel/EXP, HP/MP, arma, zona, cap).
    void dibujarHud(const Jugador& jugador, int anchoMapa, const MapMetadata& metadata,
                    const ZoneMetadata* zona, int nivelMaximoSeccion, bool limiteNivelActivo);
public:
    // Compone el frame completo y lo vuelca a la terminal.
    // Repinta todo (forceRedraw) porque el bucle del mapa limpia la pantalla
    // cada turno y el redibujado diferencial dejaría huecos.
    void renderAll(const Jugador& jugador, const Mapa& mapa, const MapMetadata& metadata,
                   const ZoneMetadata* zona, int nivelMaximoSeccion, bool limiteNivelActivo);
};

#endif