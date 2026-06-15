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

class Enemigo : public Personaje {
private:
    std::string id;            ///< Identificador único (snake_case)
    std::string asciiArt[6];   ///< 6 líneas de arte ASCII del JSON
    std::vector<Drop> botin;   ///< Array extensible de objetos+probabilidad

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
            const std::string art[6], const std::vector<Drop>& drops);

    /// Constructor copia (útil para instanciar desde plantillas)
    Enemigo(const Enemigo& copia);

    /// Acción de ataque: muestra mensaje y aplica daño al objetivo
    void atacar(Personaje* objetivo) override;

    std::string getId() const { return id; }
    const std::string* getAsciiArt() const { return asciiArt; }
    const std::vector<Drop>& getBotin() const { return botin; }
};

#endif
