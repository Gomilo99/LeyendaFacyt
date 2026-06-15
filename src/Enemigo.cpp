#include "../lib/enemigo.hpp"
#include <iostream>
#include <cstring>

Enemigo::Enemigo(std::string id, std::string nom, int hp, int atk, int def, int lvl,
                 const std::string art[6], const std::vector<Drop>& drops)
    : Personaje(nom, hp, atk, def, lvl), id(id), botin(drops) {
    for (int i = 0; i < 6; i++) asciiArt[i] = art[i];
}

Enemigo::Enemigo(const Enemigo& copia)
    : Personaje(copia), id(copia.id), botin(copia.botin) {
    for (int i = 0; i < 6; i++) asciiArt[i] = copia.asciiArt[i];
}

void Enemigo::atacar(Personaje* objetivo) {
    std::cout << nombre << " te ataca!\n";
    objetivo->recibirDano(ataque);
}
