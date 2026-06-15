#ifndef ENEMIGO_HPP
#define ENEMIGO_HPP

#include "personaje.hpp"
#include "objeto.hpp"
#include <memory>
#include <vector>
#include <string>

class Enemigo : public Personaje {
private:
    std::string id;
    std::string asciiArt[6];
    std::vector<Drop> botin;
public:
    Enemigo(std::string id, std::string nom, int hp, int atk, int def, int lvl,
            const std::string art[6], const std::vector<Drop>& drops);
    Enemigo(const Enemigo& copia);
    void atacar(Personaje* objetivo) override;

    std::string getId() const { return id; }
    const std::string* getAsciiArt() const { return asciiArt; }
    const std::vector<Drop>& getBotin() const { return botin; }
};

#endif
