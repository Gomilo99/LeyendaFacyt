#ifndef PERSONAJE_HPP
#define PERSONAJE_HPP

#include <string>
#include <iostream>
#include <algorithm>
#include "Output.hpp"

class Personaje {
protected:
    std::string nombre;
    int salud;
    int saludMaxima;
    int ataque;
    int defensa;
    int nivel;

    Output *output;
    static inline NullOutput defaultOut;

public:
    Personaje(std::string nom, int hp, int atk, int def, int lvl, Output *out = nullptr)
        : nombre(nom), salud(hp), saludMaxima(hp), ataque(atk), defensa(def), nivel(lvl),
          output(out ? out : &defaultOut) {}
    Personaje(const Personaje& copia)
        : nombre(copia.nombre), salud(copia.salud), saludMaxima(copia.saludMaxima),
          ataque(copia.ataque), defensa(copia.defensa), nivel(copia.nivel),
          output(copia.output) {}
    Personaje& operator=(const Personaje&) = default;
    virtual ~Personaje() {}
    virtual void atacar(Personaje* objetivo) = 0;

    void recibirDano(int atk_atacante) {
        int danoReal = atk_atacante - (this->defensa / 2);
        salud -= danoReal;
        output->printLine(nombre + " recibe " + std::to_string(danoReal) + " de daño!");
        //std::cout << nombre << " recibe " << danoReal << " de dano!\n";
    }

    virtual void mostrarEstado() const {
        output->printLine("\n" + nombre + " - Salud: " + std::to_string(salud) + "/" + std::to_string(saludMaxima)
            + " | Ataque: " + std::to_string(ataque) + " | Defensa: " + std::to_string(defensa));
    }

    bool estaVivo() const { return salud > 0; }
    std::string getNombre() const { return nombre; }
    void setNombre(const std::string& nuevoNombre) { nombre = nuevoNombre; }
    int getSalud() const { return salud; }
    int getSaludMaxima() const { return saludMaxima; }
    int getAtaque() const { return ataque; }
    int getDefensa() const { return defensa; }
    int getNivel() const { return nivel; }
    void setSalud(int nuevaSalud) { salud = nuevaSalud; }
    void setAtaque(int nuevoAtaque) { ataque = nuevoAtaque; }
    void setDefensa(int nuevaDefensa) { defensa = nuevaDefensa; }
    void setNivel(int nuevoNivel) { nivel = nuevoNivel; }
};

#endif
