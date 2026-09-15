#include "../lib/EnemyBehavior.hpp"
#include "../lib/Enemigo.hpp"
#include "../lib/Personaje.hpp"
#include <algorithm>

/**
 * @brief Estrategia agresiva: ataca cada turno sin condiciones.
 * @param self      Enemigo que actúa
 * @param objetivo  Jugador atacado
 * @return Mensaje de log del ataque
 */
std::string AgresivoBehavior::ejecutar(Enemigo& self, Personaje* objetivo){
    self.atacar(objetivo);
    return self.getNombre() + " te ataca!";
}

/**
 * @brief Estrategia defensiva: cura 12.5% de la vida máxima bajo el 50%.
 *
 * Al bajar del 50% de vida `self` se cura en vez de atacar (el turno de
 * curación nunca ataca). La curación se limita con `std::min` a la vida
 * máxima del enemigo.
 *
 * @param self      Enemigo que actúa
 * @param objetivo  Jugador (solo se ataca si la vida está por encima del umbral)
 * @return Mensaje de log con la acción realizada
 */
std::string DefensivoBehavior::ejecutar(Enemigo& self, Personaje* objetivo){
    if (self.getSalud() < self.getSaludMaxima() / 2) {          // por debajo del 50%
        int curacion = std::max(1, self.getSaludMaxima() / 8);  // cura 12.5%
        self.setSalud(std::min(self.getSaludMaxima(), self.getSalud() + curacion));
        return self.getNombre() + " se protege y recupera " + std::to_string(curacion) + " HP!";
    }
    self.atacar(objetivo);
    return self.getNombre() + " te ataca!";
}

/**
 * @brief Estrategia sanadora: cura 10% de la vida máxima cada turno con vida faltante.
 *
 * Solo ataca cuando está sano; si le falta vida, prescinde del ataque para
 * curarse un 10% de su vida máxima (acotado por `std::min`).
 *
 * @param self      Enemigo que actúa
 * @param objetivo  Jugador (solo se ataca si `self` está al máximo de vida)
 * @return Mensaje de log con la acción realizada
 */
std::string SanadorBehavior::ejecutar(Enemigo& self, Personaje* objetivo){
    if(self.getSalud() < self.getSaludMaxima()) {               // le falta vida
        int curacion = std::max(1, self.getSaludMaxima() / 10); // curación del 10%
        self.setSalud(std::min(self.getSaludMaxima(), self.getSalud() + curacion));
        return self.getNombre() + " se cura y recupera " + std::to_string(curacion) + " HP!";
    }
    self.atacar(objetivo);
    return self.getNombre() + " te ataca!";
}

/**
 * @brief Resuelve el nombre del JSON a una instancia de `EnemyBehavior`.
 *
 * Valores reconocidos: "defensivo" y "sanador". Cualquier otro valor
 * (incluido el default "aggressive" o un string vacío) devuelve
 * `AgresivoBehavior`.
 *
 * @param nombre Valor del campo "behavior" de `json/enemigos.json`
 * @return `shared_ptr` con la estrategia solicitada
 */
std::shared_ptr<EnemyBehavior> crearBehavior(const std::string& nombre){
    if (nombre == "defensivo") return std::make_shared<DefensivoBehavior>();
    if (nombre == "sanador") return std::make_shared<SanadorBehavior>();
    return std::make_shared<AgresivoBehavior>(); // default == aggressive
}