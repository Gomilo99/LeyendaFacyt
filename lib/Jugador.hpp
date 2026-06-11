#ifndef JUGADOR_HPP
#define JUGADOR_HPP

#include "Personaje.hpp"
#include "Objeto.hpp"
#include <map>
#include <memory>
#include <string>

class Jugador : public Personaje {
private:
    int pociones;
    std::map<std::string, int> inventario;
    std::map<std::string, std::shared_ptr<Objeto>> objetosInventario;
    std::shared_ptr<Arma> armaEquipada;
    int experiencia;
    int expNecesaria = 100;
public:
    Jugador(std::string nombre);
    Jugador(std::string nom, int hp, int atk, int def, int lvl, int poc);
    void atacar(Personaje* objetivo) override;
    int getExperiencia() const { return experiencia; }
    void setExperiencia(int nuevaExperiencia) { experiencia = nuevaExperiencia; }
    int getExperienciaNecesaria() const { return expNecesaria; }
    void setExperienciaNecesaria(int nuevaExpNecesaria) { expNecesaria = nuevaExpNecesaria; }
    int getPociones() const { return pociones; }
    void setPociones(int nuevasPociones) { pociones = nuevasPociones; }
    void usarPocion();
    void usarPocion(Objeto* pocion);
    void mostrarEstado() const override;
    void mostrarInventario();
    void agregarObjeto(std::shared_ptr<Objeto> objeto);
    void eliminarObjeto(const std::string& nombre);
    void equiparArma(std::shared_ptr<Arma> nuevaArma);
    void mostrarMenu();
    void obtenerExperiencia(int cantidad);
};

#endif
