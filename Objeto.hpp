#ifndef OBJETO_HPP
#define OBJETO_HPP

#include <string>
#include <memory>

class Objeto {
protected:
    std::string nombre;
    std::string descripcion;
public:
    Objeto(std::string nombre, std::string descripcion)
        : nombre(nombre), descripcion(descripcion) {}
    virtual ~Objeto() = default;
    virtual std::string getTipo() const { return "Generico"; }
    std::string getNombre() const { return nombre; }
    std::string getDescripcion() const { return descripcion; }
};

class Arma : public Objeto {
private:
    int dano;
public:
    Arma(std::string nombre, std::string descripcion, int dano)
        : Objeto(nombre, descripcion), dano(dano) {}
    std::string getTipo() const override { return "Arma"; }
    int getDano() const { return dano; }
    void setDano(int nuevoDano) { dano = nuevoDano; }
};

class Pocion : public Objeto {
private:
    int curacion;
public:
    Pocion(std::string nombre, std::string descripcion, int curacion)
        : Objeto(nombre, descripcion), curacion(curacion) {}
    std::string getTipo() const override { return "Pocion"; }
    int getCuracion() const { return curacion; }
    void setCuracion(int nuevaCuracion) { curacion = nuevaCuracion; }
};

class ObjClave : public Objeto {
public:
    ObjClave(std::string nombre, std::string descripcion)
        : Objeto(nombre, descripcion) {}
    std::string getTipo() const override { return "Objeto Clave"; }
};

struct Drop {
    std::shared_ptr<Objeto> objeto;
    int probabilidad;
    Drop(std::shared_ptr<Objeto> objeto, int probabilidad)
        : objeto(objeto), probabilidad(probabilidad) {}
};

#endif
