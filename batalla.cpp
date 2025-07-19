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
#include <windows.h>
// #include <unistd.h> //Linux 
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

map<string, shared_ptr<Objeto>> cargarObjetosDesdeJSON(const string& archivo){
    map<string, shared_ptr<Objeto>> objetos;
    ifstream file(archivo);

    json j;
    file >> j;
    
    // Leer armas
    if (j.contains("arma")) {
        for (const auto& item : j["arma"]) {
            string nombre = item["nombre"];
            string descripcion = item["descripcion"];
            int dano = item["dano"];
            objetos[nombre] = make_shared<Arma>(nombre, descripcion, dano);
        }
    }
    // Leer pociones
    if (j.contains("pocion")) {
        for (const auto& item : j["pocion"]) {
            string nombre = item["nombre"];
            string descripcion = item["descripcion"];
            int curacion = item["curacion"];
            objetos[nombre] = make_shared<Pocion>(nombre, descripcion, curacion);
        }
    }
    // Leer objetos clave
    if (j.contains("clave")) {
        for (const auto& item : j["clave"]) {
            string nombre = item["nombre"];
            string descripcion = item["descripcion"];
            objetos[nombre] = make_shared<ObjClave>(nombre, descripcion);
        }
    }
    file.close();
    return objetos;
}
map<string, shared_ptr<Enemigo>> cargarEnemigosDesdeJSON( const string& archivo, 
    const map<string, shared_ptr<Objeto>>& objetosDisponibles)
    {
        map<string, shared_ptr<Enemigo>> enemigos;
        ifstream file(archivo);
        json j;
        file >> j;

        for (auto it = j.begin(); it != j.end(); ++it) {
            // nivel actual (puedes guardarlo si te interesa para otra lógica)
            int nivel = stoi(it.key()); //Convierte una clave de string a int
            for (const auto& item : it.value()) {
                string nombre = item["nombre"];
                int salud = item["salud"];
                int ataque = item["ataque"];
                int defensa = item["defensa"];
                string nombreLoot1 = item["loot1"];
                string nombreLoot2 = item["loot2"];

                auto it1 = objetosDisponibles.find(nombreLoot1);
                auto it2 = objetosDisponibles.find(nombreLoot2);
                if (it1 == objetosDisponibles.end() || it2 == objetosDisponibles.end()) {
                    throw runtime_error("Objeto no encontrado: " + nombreLoot1 + " o " + nombreLoot2);
                }

                Drop drop1(it1->second, 70);
                Drop drop2(it2->second, 30);

                auto enemigo = make_shared<Enemigo>(nombre, salud, ataque, defensa, nivel, drop1, drop2);
                enemigos[nombre] = enemigo;
            }
        }
        file.close();
        return enemigos;
    }
Jugador cargarHeroe(const string& archivo){
    ifstream file(archivo);
    json j;
    file >> j;
    string nombre = j["nombre"];
    int salud = j["salud"];
    int ataque = j["ataque"];
    int defensa = j["defensa"];
    int nivel = j["nivel"];
    int pociones = j["pociones"];
    Jugador jugador(nombre, salud, ataque, defensa, nivel, pociones);
    file.close();
    return jugador;
}
shared_ptr<Enemigo> generarEnemigoPorNivel(
    const map<string, shared_ptr<Enemigo>>& enemigos, int nivelMaxPermitido){
        vector<shared_ptr<Enemigo>> candidatos;

        // Filtrar enemigos por nivel
        for (const auto& par : enemigos) {
            if (par.second->getNivel() == nivelMaxPermitido) {
                candidatos.push_back(par.second);
            }
        }

        if (candidatos.empty()) {
            throw runtime_error("No hay enemigos disponibles para el nivel solicitado");
        }

        // Selección aleatoria entre los candidatos
        int idx = rand() % candidatos.size();
        return candidatos[idx];
}

void batalla(Jugador jugador, map<string, shared_ptr<Enemigo>> enemigos){
        system("cls"); //system("clear") en Linux
    // Seleccionar un enemigo aleatorio del mapa
    
    auto enemigoSelec = generarEnemigoPorNivel(enemigos, jugador.getNivel());
    Enemigo enemigo(*enemigoSelec); // Enemigo Auxiliar
    bool jefefinal = false; // Bandera para jefe final
    if(enemigo.getNivel() == 4){
        cout << "Estas en una oscura cueva, siente una presencia extraña..." << endl;
        Sleep(3500); //sleep(5); en linux
        cout << "Ha aparecido el jefe final!!!\n Ha aparecido " << enemigo.getNombre() << endl;
        jefefinal = true;
    }else{
        cout << "Un " << enemigo.getNombre() << " ha aparecido!\n";
    }

    cout << "Comienza la batalla!\n";
    while(jugador.estaVivo() && enemigo.estaVivo()){
        jugador.mostrarMenu();
        limpiarBuffer();
        int opcion;
        cin >> opcion;
        
        switch(opcion){
            case 1:
                // enemigo es un shared_ptr<Enemigo>, usamos .get() para obtener el puntero crudo requerido por atacar
                system("cls");
                jugador.atacar(&enemigo);
                break;
            case 2:
                system("cls");
                jugador.usarPocion();
                break;
            case 3:
                jugador.mostrarInventario();
                continue;
            case 4: 
                jugador.mostrarEstado();
                continue;
            case -1: 
                cout << "Saliendo del juego...\n";
                return; // Terminar el juego si el jugador decide salir
            default:
                cout << "Opcion invalida!\n";
                continue; // Volver al menú sin hacer nada más
        }

        if (enemigo.estaVivo()) {
            enemigo.atacar(&jugador);
        }
    }

    if(!jugador.estaVivo()) {
            cout << "Has sido derrotado por " << enemigo.getNombre() << "!\n";
            return; // Terminar el juego si el jugador muere
    }
    cout << "\n\nHAS DERROTADO A '" << enemigo.getNombre() << "' !\n";
    if(jefefinal && enemigo.getSalud() <= 0){
        cout << "Felicidades mano, has derrotado al jefe final!\n";
        Sleep(3000);
        cout << "...\n";
        Sleep(3000);
        cout << "Has ganado el juego!\n";
        return; // Terminar el juego si se derrota al jefe final
    }
    int exp = jugador.getNivel() * 50; // Asumimos que el jugador obtiene 50 de experiencia por nivel
    jugador.obtenerExperiencia(exp); // Asumimos que el jugador obtiene 50 de experiencia
    // Obtener loot del enemigo
    int chance = rand() % 100;
    shared_ptr<Objeto> lootGanado = nullptr;
    
    Drop loot1 = enemigo.getLoot1();
    Drop loot2 = enemigo.getLoot2();
    if (chance < loot1.probabilidad) {
        lootGanado = loot1.objeto;
    } else if (chance < loot1.probabilidad + loot2.probabilidad) {
        lootGanado = loot2.objeto;
    }
    if (lootGanado) { // Verifica si lootGanado no es nulo
        cout << "Has obtenido: " << lootGanado->getNombre() << "\n";
        jugador.agregarObjeto(lootGanado);
    }
    
    limpiarBuffer();
    cout << "\n\nQuieres seguir jugando? (s/n) ";
    char opcion2;
    cin >> opcion2;
    if(opcion2 == 's' || opcion2 == 'S' || opcion2 <= 52 || opcion2 >= 49){
        // falta Guardar los datos del jugador en un archivo JSON
        batalla(jugador, enemigos);
    }
    return;

}
int main() {
    //Verificar si el archivo de objetos y enemigos existe
    ifstream objetosFile("objetos.json");
    ifstream enemigosFile("enemigos.json");
    ifstream heroeFile("hero.json");
    if (!objetosFile.is_open() && !enemigosFile.is_open()) {
        cerr << "Faltan archivos para ejecutar el juego.\n";
        objetosFile.close();
        enemigosFile.close();
        heroeFile.close();
        return 1;
    }
    objetosFile.close();
    enemigosFile.close();
    // Cargar objetos desde el archivo JSON
    map<string, shared_ptr<Objeto>> objetos = cargarObjetosDesdeJSON("objetos.json");
    if (objetos.empty()) {
        cerr << "No se pudieron cargar los objetos desde el archivo JSON.\n";
        heroeFile.close();
        return 1;
    }
    map<string, shared_ptr<Enemigo>> enemigos = cargarEnemigosDesdeJSON("enemigos.json", objetos);
    if (enemigos.empty()) {
        cerr << "No se pudieron cargar los enemigos desde el archivo JSON.\n";
        heroeFile.close();
        return 1;
    }

    srand(static_cast<unsigned>(time(NULL)));
    Jugador jugador("Heroe");
    if(heroeFile.is_open()) {
        jugador = cargarHeroe("heroe.json");
    }
    
    auto itEspada = objetos.find("Espada Gallo");
    if (itEspada != objetos.end()) {
        jugador.equiparArma(dynamic_pointer_cast<Arma>(itEspada->second));
    } else {
        cout << "No se encontró la espada en el inventario.\n";
    }
    batalla(jugador, enemigos);
    
    // Terminar el juego si el jugador muere
    if(!jugador.estaVivo()) {
            cout << "\n\nGAME OVER";
            return 0;
    }
    cout << "\nMuchas gracias por Jugar :)" << endl;
    heroeFile.close();
    return 0;
}