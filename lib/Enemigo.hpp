#ifndef ENEMIGO_HPP
#define ENEMIGO_HPP

/**
 * @file Enemigo.hpp
 * Clase entidad que representa un enemigo instanciado listo para combatir.
 *
 * Hereda de Personaje y añade:
 * - id:       identificador único (ej. "goblin", "admin_servidor")
 * - asciiArt: 6 líneas de arte ASCII cargadas desde el JSON
 * - botin:    vector de Drop (objeto + probabilidad), extensible
 *
 * Antes el arte se generaba por keywords en batalla.cpp; ahora viaja
 * con la instancia desde que EnemyFactory la crea.
 */

#include "personaje.hpp"
#include "objeto.hpp"
#include <memory>
#include <vector>
#include <string>
#include <utility>

class EnemyBehavior;

class Enemigo : public Personaje {
private:
    std::string id;            ///< Identificador único (snake_case)
    std::string asciiArt[6];   ///< 6 líneas de arte ASCII del JSON
    std::vector<Drop> botin;   ///< Array extensible de objetos+probabilidad
    int exp_base;
    int tier = 1;
    int displayColor = 37;
    /// Comportamiento de combate (patrón Strategy); null = ataque base (#12)
    std::shared_ptr<EnemyBehavior> comportamiento;

public:
    /**
     * Constructor principal. Copia el arte ASCII y el vector de drops.
     * @param id    Identificador único
     * @param nom   Nombre visible
     * @param hp    Salud máxima
     * @param atk   Ataque base
     * @param def   Defensa base
     * @param lvl   Nivel del enemigo
     * @param art   Array de 6 strings con arte ASCII
     * @param drops Vector de objetos con probabilidad de drop
     */
    Enemigo(std::string id, std::string nom, int hp, int atk, int def, int lvl,
            const std::string art[6], const std::vector<Drop>& drops, int exp_base);

    /**
     * Constructor copia. Propaga stats, arte, botín, tier y comportamiento
     * (el `shared_ptr<EnemyBehavior>` se comparte, no se clona; los behaviors
     * son inmutables y sin estado). Útil para instanciar desde plantillas.
     */
    Enemigo(const Enemigo& copia);

    /// Acción de ataque: muestra mensaje y aplica daño al objetivo
    void atacar(Personaje* objetivo) override;

    std::string getId() const { return id; }
    const std::string* getAsciiArt() const { return asciiArt; }
    const std::vector<Drop>& getBotin() const { return botin; }
    int getExpBase() const {return exp_base; }
    int getTier() const { return tier; }
    void setTier(int value) { tier = value; }
    int getDisplayColor() const { return displayColor; }
    void setDisplayColor(int value) { displayColor = value; }
    void aplicarMultiplicadorStats(float value);
    int experienciaCalculada() const;

    void setExpBase(int new_exp_base){ exp_base = new_exp_base; }

    /**
     * @brief Asigna la estrategia de combate del enemigo.
     * @param b Estrategia a usar (se comparte entre copias del enemigo)
     * @see EnemyBehavior para las reglas de cada comportamiento
     */
    void setComportamiento(std::shared_ptr<EnemyBehavior> b) { 
        comportamiento = std::move(b);
    }

    /**
     * @brief Ejecuta la acción del comportamiento asignado en su turno.
     * @param objetivo Personaje atacado (normalmente el jugador)
     * @return Mensaje en español para el log de combate
     * @note Si `comportamiento` es null, cae en ataque base (`atacar`).
     *       Lo llama `BattleSystem::doEnemyTurn()`.
     */
    std::string ejecutarComportamiento(Personaje* objetivo);
};

#endif
