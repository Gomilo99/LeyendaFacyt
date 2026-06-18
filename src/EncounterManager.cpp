#include "../lib/EncounterManager.hpp"
#include "../lib/DataManager.hpp"
#include <random>

/**
 * Inicializa el gestor con terreno LLANURA (probabilidad base 10%)
 * y contador de pasos en 0 (primeros pasos con gracia).
 */
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

/**
 * Probabilidad base según el terreno donde se mueve el jugador.
 * - CAMINO:    5%  (seguro)
 * - LLANURA:  10%  (neutral)
 * - MAZMORRA: 14%  (peligroso)
 * - BOSQUE:   18%  (muy peligroso)
 */
int EncounterManager::getProbabilidadBase() const {
    switch (terrenoActual) {
        case Terreno::CAMINO:    return 5;
        case Terreno::LLANURA:   return 10;
        case Terreno::BOSQUE:    return 18;
        case Terreno::MAZMORRA:  return 14;
        default:                 return 10;
    }
}

/**
 * Verifica si debe ocurrir un encuentro aleatorio.
 *
 * Reglas:
 * 1. No hay encuentros en los primeros 3 pasos (gracia).
 * 2. Probabilidad base según terreno.
 * 3. Aumenta +3% por cada paso extra después del paso 3.
 * 4. Tope máximo de 40%.
 * 5. Si el RNG acierta, resetea el contador y devuelve true.
 */
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
