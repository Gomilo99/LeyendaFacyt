#ifndef MAP_METADATA_HPP
#define MAP_METADATA_HPP

#include <map>
#include <string>
#include <vector>

struct ZoneMetadata {
    std::string id;
    int x = 0, y = 0, width = 0, height = 0;
    std::string terrain = "plain";
    int terrainColor = 32;
    std::string terrainStyle = ".";
    bool safe = false;
    float encounterMultiplier = 1.0f;
    float statMultiplier = 1.0f;
    float xpMultiplier = 1.0f;
    std::vector<std::pair<std::string, int>> enemies;
    std::string bossId;
    bool contains(int px, int py) const {
        return px >= x && py >= y && px < x + width && py < y + height;
    }
};

struct MapMetadata {
    int section = 1;
    int levelCap = 99;
    int encounterBase = 10;
    float encounterMultiplier = 1.0f;
    float encounterGrowthCap = 0.20f;
    int encounterGraceSteps = 4;
    std::map<std::string, std::pair<int, std::string>> terrainStyles;
    std::map<char, int> healing;
    std::vector<ZoneMetadata> zones;

    bool load(const std::string& path);
    const ZoneMetadata* zoneAt(int x, int y) const;
};

#endif
