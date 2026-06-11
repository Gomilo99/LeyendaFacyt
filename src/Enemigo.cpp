#include "../lib/Enemigo.hpp"
#include <iostream>

Enemigo::Enemigo(std::string nom, int hp, int atk, int def, int lvl, Drop d1, Drop d2)
    : Personaje(nom, hp, atk, def, lvl), loot1(d1), loot2(d2) {}

Enemigo::Enemigo(const Enemigo& copia)
    : Personaje(copia), loot1(copia.loot1), loot2(copia.loot2) {}

void Enemigo::atacar(Personaje* objetivo) {
    std::cout << nombre << " te ataca!\n";
    objetivo->recibirDano(ataque);
}

void Enemigo::setloot(Drop nuevoLoot1, Drop nuevoLoot2) {
    loot1 = nuevoLoot1;
    loot2 = nuevoLoot2;
}
