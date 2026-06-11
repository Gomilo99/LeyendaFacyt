#ifndef PERSONAJE_HPP
#define PERSONAJE_HPP

#include <string>
#include <iostream>
#include <algorithm>

class Personaje {
protected:
    std::string nombre;
    int salud;
    int saludMaxima;
    int ataque;
    int defensa;
    int nivel;

public:
    Personaje(std::string nom, int hp, int atk, int def, int lvl)
        : nombre(nom), salud(hp), saludMaxima(hp), ataque(atk), defensa(def), nivel(lvl) {}
    Personaje(const Personaje& copia)
        : nombre(copia.nombre), salud(copia.salud), saludMaxima(copia.saludMaxima),
          ataque(copia.ataque), defensa(copia.defensa), nivel(copia.nivel) {}
    Personaje& operator=(const Personaje&) = default;
    virtual ~Personaje() {}
    virtual void atacar(Personaje* objetivo) = 0;

    void recibirDano(int dano) {
        int danoReal = (dano <= defensa) ? 0 : std::max(1, dano - defensa);
        salud -= danoReal;
        std::cout << nombre << " recibe " << danoReal << " de dano!\n";
    }

    virtual void mostrarEstado() const {
        std::cout << "\n" << nombre << " - Salud: " << salud << "/" << saludMaxima
                  << " | Ataque: " << ataque << " | Defensa: " << defensa << std::endl;
    }

    bool estaVivo() const { return salud > 0; }
    std::string getNombre() const { return nombre; }
    int getSalud() const { return salud; }
    int getAtaque() const { return ataque; }
    int getDefensa() const { return defensa; }
    int getNivel() const { return nivel; }
    void setSalud(int nuevaSalud) { salud = nuevaSalud; }
    void setAtaque(int nuevoAtaque) { ataque = nuevoAtaque; }
    void setDefensa(int nuevaDefensa) { defensa = nuevaDefensa; }
    void setNivel(int nuevoNivel) { nivel = nuevoNivel; }
};

#endif
