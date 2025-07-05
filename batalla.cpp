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
#include "json.hpp"
//#include <curses> //Linux
//#include <conio.h> //para windows
// This project is a simple turn-based battle game in C++
// where a player can attack enemies or use potions to heal.

using namespace std;
using json = nlohmann::json;
// This project uses the nlohmann/json library for JSON parsing.

//Clases de Objetos
class Objeto {
    protected: 
    string nombre;
    string descripcion;

    public:
    Objeto(string nombre, string descripcion)
        : nombre(nombre), descripcion(descripcion) {}
        
        virtual ~Objeto() = default;
        virtual string tipo() const { return "Generico"; }
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

    string tipo() const override { return "Arma"; }
    int getDano() const { return dano; }
};
class Pocion : public Objeto {
    private:
    int curacion;

    public:
    Pocion(string nombre, string descripcion, int curacion)
        : Objeto(nombre, descripcion), curacion(curacion) {}

    string tipo() const override { return "Pocion"; }
    int getCuracion() const { return curacion; }
};
class ObjClave : public Objeto {
    public:
    ObjClave(string nombre, string descripcion)
        : Objeto(nombre, descripcion) {}
        string tipo() const override { return "Objeto Clave"; }
};

struct Drop{
    Objeto* objeto;
    int probabilidad;

    Drop(Objeto* objeto, int probabilidad)
        : objeto(objeto), probabilidad(probabilidad) {}
};


//Clases de Personajes
class Personaje{
    protected:
    string nombre;
    int salud;
    int saludMaxima;
    int ataque;
    int defensa;

    public:
    Personaje(string nombre, int salud, int ataque, int defensa)
        : nombre(nombre), salud(salud), saludMaxima(salud), ataque(ataque), defensa(defensa){}

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
            cout << nombre << " recibe " << danoReal << " de daño!\n";
        }
        
        virtual void mostrarEstado() {
            cout << nombre << " - Salud: " << salud << "/" << saludMaxima 
            << " | Ataque: " << ataque << " | Defensa: " << defensa << endl;
        }

        bool estaVivo() const { return salud > 0; }
        string getNombre() const { return nombre;}
        int getSalud() const { return salud; }
        int getAtaque() const { return ataque; }
        int getDefensa() const { return defensa; }

        };

// Clase Derivada Jugador
class Jugador : public Personaje {
    private:
    int pociones;
    map<string, int> inventario;
    shared_ptr<Arma> armaEquipada;

    public:
    Jugador(string nombre)
        : Personaje(nombre, 100, 15, 10), pociones(3), armaEquipada(NULL) {}

    void atacar(Personaje* objetivo) override {
        cout << nombre << " Atacas a " << objetivo->getNombre() << "!\n";
        objetivo->recibirDano(ataque);
    }

    void usarPocion(){
        if(pociones > 0){
            int curacion = 30;
            salud = min(salud + curacion, saludMaxima);
            pociones--;
            cout << "Usas una poción. Salud recuperdad: +" << curacion << endl;
        } else {
            cout << "No tienes pociones restantes!\n";
        }
        }

    virtual void mostrarEstado() override {
            cout << nombre << " - Salud: " << salud << "/" << saludMaxima 
            << " | Ataque: " << ataque << " | Defensa: " << defensa 
            << "\nArma equipada: " << armaEquipada->getNombre() << "| daño: " 
            << armaEquipada->getDano() << endl;
        }
    void mostrarInventario(){
        cout << "Inventario:\n";
        for (const auto& par : inventario) {
            cout << "- " << par.first << " x" << par.second << "\n";
        }
        if(armaEquipada){
            cout << "Arma equipada: " << armaEquipada->getNombre() 
                 << " (" << armaEquipada->getDano() << " de daño)\n";
        } else {
            cout << "No tienes un arma equipada.\n";
        }
    };
    
    void agregarObjeto(shared_ptr<Objeto> objeto){
        string nombre = objeto->getNombre();
        inventario[nombre]++;
        //por que? Violacion de memoria :'(

        auto arma = std::dynamic_pointer_cast<Arma>(objeto);
        if (arma) {
            cout << "Has equipado el arma: " << arma->getNombre() << " (" << arma->getDano() << " de daño).\n";
            cout << "Deseas equiparla? (s/n(: ";
        char r;
        cin >> r;
        if(r == 's' || r == 'S') {
            equiparArma(arma);
        }
    } 
    }
    void equiparArma(shared_ptr<Arma> nuevaArma) {
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
};

// Clase Derivada Enemigo
class Enemigo : public Personaje {
    private:
    Drop loot1, loot2;
    public:
    Enemigo(string nombre, int salud, int ataque, int defensa, Drop loot1, Drop loot2)
        : Personaje(nombre, salud, ataque, defensa), loot1(loot1), loot2(loot2) {}
    
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

map<string, shared_ptr<Objeto>> cargarObjetosDesdeJSON(const string& archivo){
    map<string, shared_ptr<Objeto>> objetos;
    ifstream file(archivo);
    json j;
    file >> j;
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo: " << archivo << endl;
        return objetos;
    }
    
    for (const auto& item : j) {
        string tipo = item["tipo"];
        string nombre = item["nombre"];
        string descripcion = item["descripcion"];

        if (tipo == "Arma") {
            int dano = item["dano"];
            objetos[nombre] = make_shared<Arma>(nombre, descripcion, dano);
        } else if (tipo == "Pocion") {
            int curacion = item["curacion"];
            objetos[nombre] = make_shared<Pocion>(nombre, descripcion, curacion);
        } else if (tipo == "Objeto Clave") {
            objetos[nombre] = make_shared<ObjClave>(nombre, descripcion);
        }
    }

    file.close();
    return objetos;
}
map<string, shared_ptr<Enemigo>> cargarEnemigosDesdeJSON(
    const string& archivo, 
    const map<string, shared_ptr<Objeto>>& objetosDisponibles)
    {
        map<string, shared_ptr<Enemigo>> enemigos;
        ifstream file(archivo);
        if(!file.is_open()) {
            throw std::runtime_error("No se pudo arbir la lista de enemigos");
        }

        json j;
        file >> j;

        for (const auto& item: j){
            string nombre = item["nombre"];
            int salud = item["salud"];
            int ataque = item["ataque"];
            int defensa = item["defensa"];
            string nombreLoot1 = item["loot1"];
            string nombreLoot2 = item["loot2"];

            auto it1 = objetosDisponibles.find(nombreLoot1);
            auto it2 = objetosDisponibles.find(nombreLoot2);
            if (it1 == objetosDisponibles.end() || it2 == objetosDisponibles.end()) {
                throw std::runtime_error("Objeto no encontrado: " + nombreLoot1 + " o " + nombreLoot2);
                continue; // O puedes lanzar una excepción
            }

            Drop drop1(it1->second.get(), 70);
            Drop drop2(it2->second.get(), 30);

            auto enemigo = make_shared<Enemigo>(nombre, salud, ataque, defensa, drop1, drop2);
            enemigos[nombre] = enemigo;
        }
        return enemigos;
    }

    void batalla(Jugador jugador, map<string, shared_ptr<Enemigo>> enemigos){
        system("clear");
    // Seleccionar un enemigo aleatorio del mapa
    int idx = rand() % enemigos.size();
    auto it = enemigos.begin();
    std::advance(it, idx);
    auto enemigo = it->second;


    cout << "Un " << enemigo->getNombre() << " ha aparecido!\n";

    cout << "¡Comienza la batalla!\n";
    while(jugador.estaVivo() && enemigo->estaVivo()){
        limpiarBuffer();
        jugador.mostrarMenu();
        int opcion;
        cin >> opcion;
        
        switch(opcion){
            case 1:
                // enemigo es un shared_ptr<Enemigo>, usamos .get() para obtener el puntero crudo requerido por atacar
                jugador.atacar(enemigo.get());
                break;
            case 2:
                jugador.usarPocion();
                break;
            case 3:
                jugador.mostrarInventario();
                break;
            case 4: 
                jugador.mostrarEstado();
                continue;
            default:
                cout << "Opción inválida!\n";
                continue; // Volver al menú sin hacer nada más
        }

        if (enemigo->estaVivo()) {
            enemigo->atacar(&jugador);
        }
    }

    if(!jugador.estaVivo()) {
            cout << "Has sido derrotado por " << enemigo->getNombre() << "!\n";
            return; // Terminar el juego si el jugador muere
    }
    cout << "\n\nHAS DERROTADO A " << enemigo->getNombre() << "!\n";

    // Obtener loot del enemigo
    int chance = rand() % 100;
    Objeto* lootGanado = nullptr;
    bool obtenido = false;
    Drop loot1 = enemigo->getLoot1();
    Drop loot2 = enemigo->getLoot2();
    if (chance < loot1.probabilidad) {
        lootGanado = loot1.objeto;
    } else if (chance < loot1.probabilidad + loot2.probabilidad) {
        lootGanado = loot2.objeto;
    }
    if (lootGanado) {
        cout << "Has obtenido: " << lootGanado->getNombre() << "\n";
        jugador.agregarObjeto(shared_ptr<Objeto>(lootGanado));
    }
    delete &enemigo;
    limpiarBuffer();
    cout << "\n\nQuieres seguir jugando? (s/n) ";
    char opcion2;
    cin >> opcion2;
    if(opcion2 == 's' || opcion2 == 'S'){
        batalla(jugador, enemigos);
    }
    system("clear");
    cout << "\nMuchas gracias por Jugar";
    return;

}
int main() {
    // Cargar objetos desde el archivo JSON
    map<string, shared_ptr<Objeto>> objetos = cargarObjetosDesdeJSON("objetos.json");
    if (objetos.empty()) {
        cerr << "No se pudieron cargar los objetos desde el archivo JSON.\n";
        return 1;
    }
    map<string, shared_ptr<Enemigo>> enemigos = cargarEnemigosDesdeJSON("enemigos.json", objetos);
    if (enemigos.empty()) {
        cerr << "No se pudieron cargar los enemigos desde el archivo JSON.\n";
        return 1;
    }

    srand(static_cast<unsigned>(time(NULL)));
    
    Jugador jugador("Heroe");
    auto itEspada = objetos.find("Espada Gallo");
    if (itEspada != objetos.end()) {
        jugador.equiparArma(dynamic_pointer_cast<Arma>(itEspada->second));
    } else {
        cout << "No se encontró la espada en el inventario.\n";
    }

    batalla(jugador, enemigos);
    if(!jugador.estaVivo()) {
            cout << "\n\nGAME OVER";
            return 0; // Terminar el juego si el jugador muere
    }
    
    return 0;
}