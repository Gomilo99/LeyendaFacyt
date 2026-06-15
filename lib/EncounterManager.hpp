#ifndef ENCOUNTER_MANAGER_HPP
#define ENCOUNTER_MANAGER_HPP

class EncounterManager {
public:
    enum class Terreno {
        CAMINO,
        LLANURA,
        BOSQUE,
        MAZMORRA
    };

    EncounterManager();

    void setTerreno(Terreno t);
    void registrarPaso();
    bool verificarEncuentro();
    void resetear();

private:
    Terreno terrenoActual;
    int pasosDesdeUltimo;

    int getProbabilidadBase() const;
};

#endif
