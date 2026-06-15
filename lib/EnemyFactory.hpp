#ifndef ENEMY_FACTORY_HPP
#define ENEMY_FACTORY_HPP

#include <map>
#include <vector>
#include <memory>
#include <string>
#include "enemigo.hpp"
#include "objeto.hpp"
/*
MAIN_MENU ──Enter──→ OVERWORLD ──movimiento──→ (random encounter check)
                             │                      ↓
                             │                  BATTLE ──victoria/huir──→ OVERWORLD
                             │                      ↓
                             │                  GAME_OVER (si muerte)
                             │
                             └── tile 'B' ──→ BATTLE (boss)

EnemyFactory::crearEnemigo(nivel)
  → selección ponderada por peso (weighted random)
  → devuelve Enemigo instanciado con stats + asciiArt + botin

EncounterManager::verificarEncuentro()
  → 3 pasos mínimos de gracia
  → probabilidad base 10% (llanura), 5% (camino), 18% (bosque), 14% (mazmorra)
  → +3% por paso extra (cap 40%)
*/
class EnemyFactory {
public:
    void cargarDesdeJSON(const std::string& path,
                         const std::map<std::string, std::shared_ptr<Objeto>>& objetos);

    Enemigo crearEnemigo(int nivel);
    Enemigo crearJefe(int nivel);
    bool hayJefe(int nivel) const;

private:
    struct EnemyTemplate {
        std::string id;
        std::string nombre;
        int salud;
        int ataque;
        int defensa;
        int nivel;
        int peso;
        std::string asciiArt[6];
        std::vector<Drop> botin;
        bool boss;
    };

    std::map<int, std::vector<EnemyTemplate>> plantillas;

    const EnemyTemplate& seleccionarPlantilla(int nivel);
    int totalPeso(int nivel) const;
};

#endif
