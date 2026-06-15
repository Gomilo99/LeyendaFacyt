#include "../lib/EncounterManager.hpp"
#include "../lib/DataManager.hpp"
#include <random>

EncounterManager::EncounterManager()
    : terrenoActual(Terreno::LLANURA), pasosDesdeUltimo(0) {}

void EncounterManager::setTerreno(Terreno t) {
    terrenoActual = t;
}

void EncounterManager::registrarPaso() {
    pasosDesdeUltimo++;
}

void EncounterManager::resetear() {
    pasosDesdeUltimo = 0;
}

int EncounterManager::getProbabilidadBase() const {
    switch (terrenoActual) {
        case Terreno::CAMINO:    return 5;
        case Terreno::LLANURA:   return 10;
        case Terreno::BOSQUE:    return 18;
        case Terreno::MAZMORRA:  return 14;
        default:                 return 10;
    }
}

bool EncounterManager::verificarEncuentro() {
    if (pasosDesdeUltimo < 3) return false;

    int probBase = getProbabilidadBase();
    int prob = probBase + (pasosDesdeUltimo - 3) * 3;
    if (prob > 40) prob = 40;

    std::uniform_int_distribution<int> dist(0, 99);
    if (dist(DataManager::rng()) < prob) {
        resetear();
        return true;
    }
    return false;
}
