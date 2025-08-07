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
#include "objeto.hpp"
using namespace std;

//Clases de Personajes
class Personaje{
    protected:
    string nombre;
    int salud;
    int saludMaxima;
    int ataque;
    int defensa;
    int nivel;

    public:
    Personaje(string nom, int hp, int atk, int def, int lvl)
        : nombre(nom), salud(hp), saludMaxima(hp), ataque(atk), defensa(def), nivel(lvl) {}
    Personaje(const Personaje& copia) 
        : nombre(copia.nombre), salud(copia.salud), saludMaxima(copia.saludMaxima),
            ataque(copia.ataque), defensa(copia.defensa), nivel(copia.nivel) {}
        virtual ~Personaje() {}
        virtual void atacar(Personaje* objetivo) = 0;

        void recibirDano(int dano){
            int danoReal;
            if(dano <= defensa){
                danoReal = 0;
            }else{
                danoReal = max(1, dano - defensa);
            }
            salud -= danoReal;
            cout << nombre << " recibe " << danoReal << " de dano!\n";
        }
        
        virtual void mostrarEstado() {
            cout << "\n" << nombre << " - Salud: " << salud << "/" << saludMaxima 
            << " | Ataque: " << ataque << " | Defensa: " << defensa << endl;
        }

        bool estaVivo() const { return salud > 0; }
        string getNombre() const { return nombre;}
        int getSalud() const { return salud; }
        int getAtaque() const { return ataque; }
        int getDefensa() const { return defensa; }
        int getNivel() const { return nivel; }

        void setSalud(int nuevaSalud) { salud = nuevaSalud; }
        void setAtaque(int nuevoAtaque) { ataque = nuevoAtaque; }
        void setDefensa(int nuevaDefensa) { defensa = nuevaDefensa; }
        void setNivel(int nuevoNivel) { nivel = nuevoNivel; }


        };

// Clase Derivada Jugador
class Jugador : public Personaje {
    private:
    int pociones;
    map<string, int> inventario; // nombre -> cantidad
    map<string, shared_ptr<Objeto>> objetosInventario;// nombre -> objeto
    shared_ptr<Arma> armaEquipada;
    int experiencia; // Experiencia del jugador
    int expNecesaria = 100; // Experiencia necesaria para subir de nivel
    public:
    Jugador(string nombre)
        : Personaje(nombre, 100, 15, 10, 1), pociones(3), armaEquipada(NULL), experiencia(0) {}
    Jugador(string nom, int hp, int atk, int def, int lvl, int poc)
        : Personaje(nom, hp, atk, def, lvl), pociones(poc), armaEquipada(NULL), experiencia(0)
        {}

    void atacar(Personaje* objetivo) override {
        cout << nombre << " Atacas a " << objetivo->getNombre() << "!\n";
        objetivo->recibirDano(ataque);
    }
    int getExperiencia() const { return experiencia; }
    void setExperiencia(int nuevaExperiencia) { experiencia = nuevaExperiencia; }
    int getExperienciaNecesaria() const { return expNecesaria; }
    void setExperienciaNecesaria(int nuevaExpNecesaria) { expNecesaria = nuevaExpNecesaria; }

    int getPociones() const { return pociones; }
    void setPociones(int nuevasPociones) { pociones = nuevasPociones; }


    void usarPocion(){
        if(pociones > 0){
            int curacion = 30;
            salud = min(salud + curacion, saludMaxima);
            pociones--;
            cout << "Usas una pocion. Salud recuperdad: +" << curacion << endl;
        } else {
            cout << "No tienes pociones restantes!\n";
        }
        }
    void usarPocion(Objeto* pocion){
        auto pocionPtr = dynamic_cast<Pocion*>(pocion);
        if (pocionPtr) {
            int curacion = pocionPtr->getCuracion();
            salud = min(salud + curacion, saludMaxima);
            cout << "\n" << "Salud recuperada: +" << curacion << endl;
        } else {
            cout << "\nEl objeto no es una pocion valida.\n";
        }
    }
    virtual void mostrarEstado() override {
            cout << "\n" << nombre << " - Salud: " << salud << "/" << saludMaxima 
            << " | Ataque: " << ataque << " | Defensa: " << defensa 
            << "\nArma equipada: " << armaEquipada->getNombre() << "| dano: " 
            << armaEquipada->getDano() << "\nNivel: " << nivel <<" | Experiencia: " << experiencia 
            << "/" << expNecesaria << endl;
    }
    void mostrarInventario(){
        cout << "Inventario:\n";
        for (const auto& par : inventario) {
            cout << "- " << par.first << " x" << par.second << "\n"
            << objetosInventario[par.first]->getDescripcion() << endl;
        }
        if(armaEquipada){
            cout << "\nArma equipada: " << armaEquipada->getNombre() 
                 << " (" << armaEquipada->getDano() << " de dano)\n" 
                 << armaEquipada->getDescripcion() << endl;
        } else {
            cout << "No tienes un arma equipada.\n";
        }
        string seleccion;
        cout << "Deseas usar un objeto? (s/n): ";
        cin >> seleccion;
        cout << endl;
        if(seleccion == "s" || seleccion == "S") {
            string nombreObjeto;
            cout << "\nIngresa el nombre del objeto: ";
            cin >> nombreObjeto;
            auto itObj = objetosInventario.find(nombreObjeto);
            if (itObj != objetosInventario.end()) {
                usarPocion(itObj->second.get());
                eliminarObjeto(nombreObjeto);
            } else {
                cout << "No tienes ese objeto en tu inventario.\n";
            }
        }
    }
    void agregarObjeto(shared_ptr<Objeto> objeto){
        string nombre = objeto->getNombre();
        inventario[nombre]++;
        objetosInventario[nombre] = objeto;

        auto arma = dynamic_pointer_cast<Arma>(objeto);
        if (arma) {
            cout << "Has encontrado el arma: " << arma->getNombre() << " (" << arma->getDano() << " de daño).\n";
            cout << "¿Deseas equiparla? (s/n): ";
            char r;
            cin >> r;
            if(r == 's' || r == 'S') {
                equiparArma(arma);
            }
        }
    }
    void eliminarObjeto(const string& nombre){
        auto it = inventario.find(nombre);
        if (it != inventario.end()) {
            it->second -= 1;
            if (it->second <= 0) {
                inventario.erase(it);
                objetosInventario.erase(nombre);
            }
            //cout << "Has usado: " << nombre << " .\n";
        } else {
            cout << "No tienes ese objeto en tu inventario.\n";
        }
    };
    
    void equiparArma(shared_ptr<Arma> nuevaArma){
        if(armaEquipada){
            ataque -= armaEquipada->getDano();
        }
        armaEquipada = nuevaArma;
        ataque += nuevaArma->getDano();
        cout << "Has equipado el arma: " << nuevaArma->getNombre() << "\n";
        }

    void mostrarMenu(){
        cout << "\n--- Turno del Jugador ---\n";
        cout << "1. Atacar\n";
        cout << "2. Usar pocion (" << pociones << " restantes)\n";
        cout << "3. Ver inventario\n";
        cout << "4. Ver estado\n";
        cout << "Elige una opcion: ";
    }
    void obtenerExperiencia(int cantidad) {
        experiencia += cantidad;
        cout << "Has ganado " << cantidad << " de experiencia!\n";

        if(experiencia >= expNecesaria){
            cout << "Has subido de nivel!\n";
            saludMaxima += 50*(nivel + 1); // Aumentar salud máxima al subir de nivel
            salud = saludMaxima; // Restaurar salud al subir de nivel
            defensa += 5*(nivel + 1); // Aumentar defensa al subir de nivel
            ataque += 5*(nivel + 1); // Aumentar ataque al subir de nivel
            nivel++;
            expNecesaria += 200;
            if(nivel == 3) expNecesaria = 700;

            cout << "Subida de Estadisticas!!\n";
            cout << "Nivel: " << nivel << " | Salud: " << salud << "/" << saludMaxima 
                 << " | Ataque: " << ataque << " | Defensa: " << defensa 
                 << " | Experiencia: " << experiencia << "/" << expNecesaria << endl;
        }
    }

};

// Clase Derivada Enemigo
class Enemigo : public Personaje {
    private:
    Drop loot1, loot2;
    public:
    Enemigo(string nom, int hp, int atk, int def, int lvl, Drop d1, Drop d2)
        : Personaje(nom, hp, atk, def, lvl), loot1(d1), loot2(d2) {}
    Enemigo(const Enemigo& copia)
        : Personaje(copia), loot1(copia.loot1), loot2(copia.loot2) {}

    void atacar(Personaje* objetivo) override {
        cout << nombre << " te ataca!\n";
        objetivo->recibirDano(ataque);
    }

    void setloot(Drop nuevoLoot1, Drop nuevoLoot2) {
        loot1 = nuevoLoot1;
        loot2 = nuevoLoot2;
    }
    Drop getLoot1() const { return loot1; }
    Drop getLoot2() const { return loot2; }
};

void limpiarBuffer(){
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}