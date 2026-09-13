#include "../lib/EncounterManager.hpp"
#include "../lib/DataManager.hpp"
#include <random>

/**
 * Inicializa el gestor con terreno LLANURA (probabilidad base 10%)
 * y contador de pasos en 0 (primeros pasos con gracia).
 */
EncounterManager::EncounterManager()
    : terrenoActual(Terreno::LLANURA), pasosDesdeUltimo(0),
      probabilidadBase(10), multiplicador(1.0f), topeCrecimiento(0.20f),
      pasosGracia(4), terrenoSeguro(false) {}

void EncounterManager::setTerreno(Terreno t) {
    terrenoActual = t;
}

void EncounterManager::registrarPaso() {
    pasosDesdeUltimo++;
}

void EncounterManager::resetear() {
    pasosDesdeUltimo = 0;
}
void EncounterManager::configurar(int base, float multiplier, float growthCap,
                                  int graceSteps, bool safe) {
    probabilidadBase = base;
    multiplicador = multiplier;
    topeCrecimiento = growthCap;
    pasosGracia = graceSteps;
    terrenoSeguro = safe;
}
void EncounterManager::setSeguro(bool safe) { terrenoSeguro = safe; }

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
    if (terrenoSeguro || pasosDesdeUltimo <= pasosGracia) return false;

    int probBase = getProbabilidadBase();
    int prob = static_cast<int>(probBase * multiplicador);
    int growth = static_cast<int>(probBase * topeCrecimiento);
    int stepGrowth = (pasosDesdeUltimo - pasosGracia) * 3;
    if (stepGrowth > growth) stepGrowth = growth;
    prob += stepGrowth;
    if (prob > static_cast<int>(probBase * multiplicador) + growth)
        prob = static_cast<int>(probBase * multiplicador) + growth;

    std::uniform_int_distribution<int> dist(0, 99);
    if (dist(DataManager::rng()) < prob) {
        resetear();
        return true;
    }
    return false;
}
