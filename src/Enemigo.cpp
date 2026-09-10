#include "../lib/Enemigo.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>

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
                const std::string art[6], const std::vector<Drop>& drops, int exp_base)
    : Personaje(nom, hp, atk, def, lvl), id(id), botin(drops), exp_base(exp_base) {
    for (int i = 0; i < 6; i++) asciiArt[i] = art[i];
}

/**
 * Constructor copia. Útil para instanciar un Enemigo desde
 * una plantilla almacenada en EnemyFactory.
 */
Enemigo::Enemigo(const Enemigo& copia)
    : Personaje(copia), id(copia.id), botin(copia.botin),
    exp_base(copia.exp_base), tier(copia.tier), xpMultiplier(copia.xpMultiplier) {
    for (int i = 0; i < 6; i++) asciiArt[i] = copia.asciiArt[i];
}
void Enemigo::aplicarMultiplicadorStats(float value) {
    saludMaxima = std::max(1, static_cast<int>(saludMaxima * value));
    salud = saludMaxima;
    ataque = std::max(1, static_cast<int>(ataque * value));
    defensa = std::max(0, static_cast<int>(defensa * value));
}
int Enemigo::experienciaCalculada() const {
    const int tierBonus = std::max(1, tier);
    return std::max(1, static_cast<int>(50.0f * nivel * tierBonus * xpMultiplier));
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
