#include "../lib/Enemigo.hpp"
#include <iostream>
#include <cstring>

/**
 * Constructor principal. Inicializa stats base desde Personaje,
 * asigna el identificador único, copia las 6 líneas de arte ASCII
 * y el vector de drops (botín).
 *
 * @param id    Identificador único (snake_case, ej. "goblin")
 * @param nom   Nombre visible en combate
 * @param hp    Salud máxima
 * @param atk   Daño base
 * @param def   Defensa base
 * @param lvl   Nivel del enemigo
 * @param art   Array de 6 strings con arte ASCII
 * @param drops Vector de objetos con probabilidad de drop
 */
Enemigo::Enemigo(std::string id, std::string nom, int hp, int atk, int def, int lvl,
                 const std::string art[6], const std::vector<Drop>& drops)
    : Personaje(nom, hp, atk, def, lvl), id(id), botin(drops) {
    for (int i = 0; i < 6; i++) asciiArt[i] = art[i];
}

/**
 * Constructor copia. Útil para instanciar un Enemigo desde
 * una plantilla almacenada en EnemyFactory.
 */
Enemigo::Enemigo(const Enemigo& copia)
    : Personaje(copia), id(copia.id), botin(copia.botin) {
    for (int i = 0; i < 6; i++) asciiArt[i] = copia.asciiArt[i];
}

/**
 * Acción de ataque del enemigo.
 * Muestra mensaje en consola y aplica el daño (ataque base)
 * al objetivo (jugador) a través de Personaje::recibirDano.
 */
void Enemigo::atacar(Personaje* objetivo) {
    std::cout << nombre << " te ataca!\n";
    objetivo->recibirDano(ataque);
}
