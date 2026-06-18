#ifndef ENCOUNTER_MANAGER_HPP
#define ENCOUNTER_MANAGER_HPP

/**
 * @file EncounterManager.hpp
 * Gestor de encuentros aleatorios por pasos.
 *
 * Cada vez que el jugador se mueve en el overworld, se registra un paso
 * y se tira un dado contra una probabilidad que depende del terreno.
 * La probabilidad aumenta gradualmente cuantos más pasos sin encuentro.
 *
 * Terrenos y sus probabilidades base:
 * - CAMINO:    5%  (rutas seguras)
 * - LLANURA:  10%  (terreno neutral)
 * - MAZMORRA: 14%  (calabozos)
 * - BOSQUE:   18%  (alta peligrosidad)
 *
 * Fórmula: prob = probBase + (pasos - 3) * 3, tope 40%
 * Hay 3 pasos mínimos de gracia tras cada encuentro.
 */

class EncounterManager {
public:
    /// Tipos de terreno con diferente probabilidad de encuentro
    enum class Terreno {
        CAMINO,   ///< 5%  — Rutas seguras
        LLANURA,  ///< 10% — Terreno neutral
        BOSQUE,   ///< 18% — Zonas densas, alta peligrosidad
        MAZMORRA  ///< 14% — Calabozos subterráneos
    };

    EncounterManager();

    /// Cambia el terreno actual (afecta la probabilidad base)
    void setTerreno(Terreno t);

    /// Incrementa el contador de pasos desde el último encuentro
    void registrarPaso();

    /**
     * Verifica si debe ocurrir un encuentro aleatorio.
     * Usa RNG con probabilidad escalable según pasos transcurridos.
     * @return true si se dispara un encuentro
     */
    bool verificarEncuentro();

    /// Resetea el contador de pasos (llamado tras un encuentro)
    void resetear();

private:
    Terreno terrenoActual;    ///< Terreno donde se mueve el jugador
    int pasosDesdeUltimo;     ///< Pasos desde el último encuentro

    /// Devuelve la probabilidad base según el terreno actual
    int getProbabilidadBase() const;
};

#endif
