#### Leyenda de tiles:

|Símbolo|Significado                |
|-------|---------------------------|
|#      |Pared — no transitable     |
|.      |Suelo — transitable        |
|P      |Posición inicial del jugador|
|E	    |Spawn de enemigo           |
|K	    |Llave mágica (victoria)    |
|H	    |Poción en el suelo         |

### Sistema de caché para no modificar archivos originales
#### Concepto
Los archivos originales (``json/objetos.json, enemigos.json, mapas/nivel1.txt``) son read-only para el juego. Cualquier modificación en tiempo de juego se guarda en ``cache/``:

```text
proyecto/
├── json/              ← originales (lectura)
│   ├── objetos.json
│   ├── enemigos.json
│   └── heroe.json     ← se deja de escribir aquí
├── mapas/             ← originales (lectura)
│   └── nivel1.txt
└── cache/             ← generado en tiempo de juego (escritura)
    ├── heroe.json     ← guardado del héroe
    ├── mapa_cache.txt ← estado actualizado del mapa (con E/H marcados como .)
    └── partida.json   ← metadatos de la partida
```

#### Flujo de carga
main():
1. Cargar JSONs desde json/ (originales, read-only)
2. Cargar mapa desde mapas/nivel1.txt (original, read-only)
3. Si existe cache/heroe.json → cargar héroe desde caché
4. Si existe cache/mapa_cache.txt → cargar mapa desde caché
   (así se conservan los cambios: E eliminados, H recogidas)
5. Si no existe caché → copiar originales a caché

#### Cómo implementar la caché del mapa
Paso 1 — Crear **CacheManager** (``lib/CacheManager.hpp``)
```cpp
namespace CacheManager {
    bool existePartida();
    void crearPartida(const Mapa& mapa, const Jugador& jugador);
    
    // Mapa
    bool guardarMapa(const Mapa& mapa);
    bool cargarMapa(Mapa& mapa);  // si falla, cargar del original
    
    // Héroe
    void guardarHeroe(const Jugador& jugador);
    Jugador cargarHeroe();
    
    // Limpiar caché (para nueva partida)
    void limpiar();
}
```
Paso 2 — Flujo en ``main()``
```
1. Cargar archivos originales (JSON, mapa)
2. if (existePartida()) {
       cargarMapa desde caché      → refleja E/H ya gastados
       cargarHeroe desde caché
   } else {
       crearPartida()              → copia originales a caché
   }
3. Bucle de juego:
   - Al vencer enemigo en 'E':
       mapa.setTile(x, y, '.')     → modifica en memoria
       CacheManager::guardarMapa(mapa)  → persiste el cambio
   - Al recoger poción en 'H':
       mapa.setTile(x, y, '.')
       CacheManager::guardarMapa(mapa)
       jugador.usarPocion()
   - Al salir del juego o girar turno:
       CacheManager::guardarHeroe(jugador)
```

Paso 3 — Formato de la caché del mapa

El ``mapa_cache.txt`` puede ser idéntico al original (``vector<string>``), solo que con los tiles modificados. La clase ``Mapa`` ya tiene ``getTile/setTile``. Solo falta un método ``guardar(archivo)``:
```cpp
// En Mapa.hpp
bool guardar(const std::string& archivo) const;

// En Mapa.cpp
bool Mapa::guardar(const std::string& archivo) const {
    std::ofstream file(archivo);
    if (!file.is_open()) return false;
    for (const auto& linea : grid) {
        file << linea << '\n';
    }
    return true;
}
```
3. Cuándo se guarda cada cosa
Evento	Se guarda en caché
Derrotar enemigo (tile 'E' → '.')	mapa_cache.txt
Recoger poción (tile 'H' → '.')	mapa_cache.txt
Cambiar de nivel / sala	mapa_cache.txt + heroe.json
Recibir daño en mapa (trampa 'T')	heroe.json
Subir de nivel en combate	heroe.json (ya lo hace guardarHeroe en batalla)
Salir del juego con 'Q'	heroe.json + mapa_cache.txt