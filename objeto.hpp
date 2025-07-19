#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <limits>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <cctype>
using namespace std;
//Clases de Objetos
class Objeto {
    protected: 
    string nombre;
    string descripcion;

    public:
    Objeto(string nombre, string descripcion)
        : nombre(nombre), descripcion(descripcion) {}
        
        virtual ~Objeto() = default;
        virtual string getTipo() const { return "Generico"; }
        string getNombre() const { return nombre; }
        string getDescripcion() const { return descripcion; }
        
};
//Clases Derivada Arma, Pocion y ObjClave
class Arma : public Objeto {
    private:
    int dano;

    public:
    Arma(string nombre, string descripcion, int dano)
        : Objeto(nombre, descripcion), dano(dano) {}

    string getTipo() const override { return "Arma"; }
    int getDano() const { return dano; }
    void setDano(int nuevoDano) {
        dano = nuevoDano;
    }
};
class Pocion : public Objeto {
    private:
    int curacion;

    public:
    Pocion(string nombre, string descripcion, int curacion)
        : Objeto(nombre, descripcion), curacion(curacion) {}

    string getTipo() const override { return "Pocion"; }
    int getCuracion() const { return curacion; }
    void setCuracion(int nuevaCuracion) {
        curacion = nuevaCuracion;
    }
};
class ObjClave : public Objeto {
    public:
    ObjClave(string nombre, string descripcion)
        : Objeto(nombre, descripcion) {}
        string getTipo() const override { return "Objeto Clave"; }
};

struct Drop{
    shared_ptr<Objeto> objeto;
    int probabilidad;

    Drop(shared_ptr<Objeto> objeto, int probabilidad)
        : objeto(objeto), probabilidad(probabilidad) {}
};