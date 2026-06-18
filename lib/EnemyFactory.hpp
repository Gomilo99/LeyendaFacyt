#ifndef ENEMY_FACTORY_HPP
#define ENEMY_FACTORY_HPP

/**
 * @file EnemyFactory.hpp
 * Fábrica de enemigos: lee el JSON, almacena plantillas por nivel
 * y crea instancias de Enemigo listas para combate.
 *
 * La selección de enemigos es ponderada según el campo "peso":
 * un enemigo con peso 10 aparece el doble que uno con peso 5.
 *
 * Los jefes (boss: true) se buscan desde el nivel actual hacia abajo
 * para soportar múltiples niveles sin jefe definido.
 */

#include <map>
#include <vector>
#include <memory>
#include <string>
#include "enemigo.hpp"
#include "objeto.hpp"

class EnemyFactory {
public:
    /**
     * Carga todas las plantillas desde el archivo JSON.
     * @param path  Ruta al archivo enemigos.json
     * @param objetos  Mapa de objetos cargados (para resolver botín)
     * @throws std::runtime_error si un objeto del botín no existe
     */
    void cargarDesdeJSON(const std::string& path,
                         const std::map<std::string, std::shared_ptr<Objeto>>& objetos);

    /**
     * Crea un enemigo aleatorio del nivel indicado.
     * La selección usa el peso de cada enemigo como probabilidad relativa.
     * @param nivel  Nivel del jugador (determina qué plantillas usar)
     * @return Enemigo instanciado con stats, arte y botín copiados
     * @throws std::runtime_error si no hay enemigos para ese nivel
     */
    Enemigo crearEnemigo(int nivel);

    /**
     * Crea el jefe del nivel (el primer boss:true desde nivel hacia abajo).
     * @param nivel  Nivel del jugador
     * @return Enemigo instanciado con datos del jefe
     * @throws std::runtime_error si no hay jefe definido
     */
    Enemigo crearJefe(int nivel);

    /**
     * Verifica si existe un jefe definido para el nivel (o inferiores).
     * @param nivel  Nivel máximo a buscar
     * @return true si hay algún enemigo con boss:true entre 1 y nivel
     */
    bool hayJefe(int nivel) const;

private:
    /**
     * Plantilla interna con todos los datos necesarios para
     * instanciar un Enemigo. Se almacena en el mapa plantillas.
     */
    struct EnemyTemplate {
        std::string id;          ///< Identificador único (snake_case)
        std::string nombre;      ///< Nombre visible en combate
        int salud;               ///< HP máximo
        int ataque;              ///< Daño base
        int defensa;             ///< Reducción de daño
        int nivel;               ///< Nivel al que pertenece
        int peso;                ///< Probabilidad relativa de aparición
        std::string asciiArt[6]; ///< 6 líneas de arte ASCII
        std::vector<Drop> botin; ///< Array de objetos con probabilidad
        bool boss;              ///< true si es el jefe del nivel
        int exp_base;           ///< Experiencia Base del enemigo
    };

    /// Almacén principal: nivel -> vector de plantillas
    std::map<int, std::vector<EnemyTemplate>> plantillas;

    /**
     * Selecciona una plantilla mediante algoritmo de ruleta ponderada.
     * Suma todos los pesos, genera un número aleatorio y resta pesos
     * hasta encontrar la plantilla ganadora.
     * @param nivel  Nivel del que seleccionar
     * @return Referencia a la plantilla seleccionada
     */
    const EnemyTemplate& seleccionarPlantilla(int nivel);

    /**
     * Suma todos los pesos de los enemigos de un nivel.
     * @param nivel  Nivel a consultar
     * @return Suma total de pesos (0 si el nivel no existe)
     */
    int totalPeso(int nivel) const;
};

#endif
