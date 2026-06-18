#ifndef JUGADOR_HPP
#define JUGADOR_HPP

#include "Personaje.hpp"
#include "Objeto.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

class Jugador : public Personaje {
private:
    int pociones;
    int mana, manaMaxima;
    std::map<std::string, int> inventario;
    std::map<std::string, std::shared_ptr<Objeto>> objetosInventario;
    std::shared_ptr<Arma> armaEquipada;
    int experiencia;
    int expNecesaria = 100;
    bool haGanado = false;
    int posX, posY;
public:
    Jugador(std::string nombre);
    Jugador(std::string nom, int hp, int atk, int def, int lvl, int poc);

    int getExperiencia() const { return experiencia; }
    void setExperiencia(int nuevaExperiencia) { experiencia = nuevaExperiencia; }
    
    int getExperienciaNecesaria() const { return expNecesaria; }
    void setExperienciaNecesaria(int nuevaExpNecesaria) { expNecesaria = nuevaExpNecesaria; }
    
    int getPociones() const { return pociones; }
    void setPociones(int nuevasPociones) { pociones = nuevasPociones; }
    
    int getMana() const { return mana; }
    int getManaMaxima() const { return manaMaxima; }
    void setMana(int nuevaMana) { mana = nuevaMana; }
    
    bool getHaGanado() const { return haGanado; }
    void setHaGanado(bool ganado) { haGanado = ganado; }

    const std::map<std::string, int>& getInventario() const { return inventario; }
    const std::map<std::string, std::shared_ptr<Objeto>>& getObjetosInventario() const { return objetosInventario; }

    int getPosX() const { return posX; }
    int getPosY() const { return posY; }
    void setPos(int x, int y) { posX = x; posY = y; }

    void atacar(Personaje* objetivo) override;
    void usarPocion();
    void usarPocion(Objeto* pocion);
    void usarMagia(Personaje* objetivo);

    void mostrarEstado() const override;
    void mostrarInventario();

    std::string getArmaNombre() const {
        return armaEquipada ? armaEquipada->getNombre() : "ninguna";
    }
    int getArmaDano() const {
        return armaEquipada ? armaEquipada->getDano() : 0;
    }

    void agregarObjeto(std::shared_ptr<Objeto> objeto);
    void agregarObjetoSilencioso(std::shared_ptr<Objeto> objeto);
    void eliminarObjeto(const std::string& nombre);
    void equiparArma(std::shared_ptr<Arma> nuevaArma, bool silencioso = false);

    // Método de envío de inventario para el renderizador
    std::vector<std::pair<std::string, std::shared_ptr<Objeto> > > getItemsList() const;

    void obtenerExperiencia(int cantidad);
};

#endif
