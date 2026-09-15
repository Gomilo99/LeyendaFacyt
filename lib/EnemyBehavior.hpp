#ifndef ENEMY_BEHAVIOR
#define ENEMY_BEHAVIOR

#include <string>
#include <memory>

class Personaje;
class Enemigo;

/**
 * @file EnemyBehavior.hpp
 * Comportamientos de combate de los enemigos (patrón Strategy).
 *
 * Cada enemigo posee un `EnemyBehavior` que decide su acción en cada turno
 * (atacar, curarse, protegerse). La estrategia se selecciona desde
 * `json/enemigos.json` mediante el campo opcional "behavior" y se asigna en
 * `EnemyFactory` a través de `Enemigo::setComportamiento`.
 *
 * Reglas de diseño:
 * - Las estrategias son **inmutables y sin estado**: una misma instancia
 *   puede compartirse entre varios enemigos sin riesgo (el `shared_ptr` del
 *   copy-constructor de `Enemigo` la propaga tal cual).
 * - El turno de curación nunca ataca: el heal paga con su DPS.
 * - La curación nunca supera la vida máxima del enemigo.
 *
 * Auditoría #12: antes todos los enemigos atacaban igual
 * (`atacar(player)`); ahora `BattleSystem::doEnemyTurn()` delega en
 * `Enemigo::ejecutarComportamiento()`.
 */

/**
 * @brief Interfaz de estrategia de combate para un enemigo.
 *
 * Define la acción que ejecuta el enemigo en su turno y el mensaje que la
 * UI muestra en el log de combate.
 */
class EnemyBehavior{
public:
    virtual ~EnemyBehavior() = default;

    /**
     * @brief Ejecuta la acción del enemigo en su turno.
     * @param self      Enemigo que actúa (debe LLAMAR los setters de Personaje
     *                  si cura o modifica su estado)
     * @param objetivo  Personaje atacado (normalmente el jugador)
     * @return Mensaje en español para el log de combate
     */
    virtual std::string ejecutar(Enemigo& self, Personaje* objetivo) = 0;
};

/**
 * @brief Estrategia por defecto: ataca cada turno.
 *
 * Aplica el `ataque` base del enemigo vía `Enemigo::atacar`, sin condiciones
 * ni alternativas. Es el comportamiento que tenían todos los enemigos antes
 * del patrón Strategy (auditoría #12).
 */
class AgresivoBehavior : public EnemyBehavior {
public:
    std::string ejecutar(Enemigo& self, Personaje* objetivo) override;
};

/**
 * @brief Estrategia defensiva: se cura cuando su vida baja del 50%.
 *
 * Por debajo del umbral (50% de la vida máxima) cura un 12.5% por turno a
 * cambio de renunciar al ataque. Por encima del umbral ataca normal.
 */
class DefensivoBehavior : public EnemyBehavior {
public:
    std::string ejecutar(Enemigo& self, Personaje* objetivo) override;
};

/**
 * @brief Estrategia sanadora: se cura siempre que le falte vida.
 *
 * Estando por debajo de su vida máxima cura un 10% por turno sin atacar;
 * estando sano ataca normal. Ideal para enemigos con mucha vida jugando a
 * "no morir", pero su DPS es bajo porque alterna curación.
 */
class SanadorBehavior : public EnemyBehavior {
public:
    std::string ejecutar(Enemigo& self, Personaje* objetivo) override;
};

/**
 * @brief Fábrica de estrategias según el nombre leído del JSON.
 * @param nombre Valor del campo "behavior": "defensivo", "sanador"
 * @return Instancia de la estrategia correspondiente
 * @note Cualquier nombre desconocido produce `AgresivoBehavior` (default).
 */
std::shared_ptr<EnemyBehavior> crearBehavior(const std::string& nombre);

#endif