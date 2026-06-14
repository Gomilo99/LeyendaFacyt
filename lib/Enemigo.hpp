#ifndef ENEMIGO_HPP
#define ENEMIGO_HPP

#include "personaje.hpp"
#include "objeto.hpp"
#include <memory>

class Enemigo : public Personaje {
private:
    Drop loot1, loot2;
public:
    Enemigo(std::string nom, int hp, int atk, int def, int lvl, Drop d1, Drop d2);
    Enemigo(const Enemigo& copia);
    void atacar(Personaje* objetivo) override;
    void setLoot(Drop nuevoLoot1, Drop nuevoLoot2);
    Drop getLoot1() const { return loot1; }
    Drop getLoot2() const { return loot2; }
};

#endif
