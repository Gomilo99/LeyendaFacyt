#ifndef MAP_METADATA_HPP
#define MAP_METADATA_HPP

#include <map>
#include <string>
#include <vector>

struct ZoneMetadata {
    std::string id;
    char tile = '\0';
    std::string terrain = "plain";
    int terrainColor = 32;
    bool safe = false;
    float encounterMultiplier = 1.0f;
    float statMultiplier = 1.0f;
    float xpMultiplier = 1.0f;
    std::vector<std::pair<std::string, int>> enemies;
    std::string bossId;
    char restoreTile = '\0';
};

struct MapMetadata {
    int section = 1;
    int levelCap = 99;
    int encounterBase = 10;
    float encounterMultiplier = 1.0f;
    float encounterGrowthCap = 0.20f;
    int encounterGraceSteps = 4;
    std::map<int, int> tierColors;
    std::map<char, int> healing;
    std::vector<ZoneMetadata> zones;

    bool load(const std::string& path);
    const ZoneMetadata* zoneAt(char tile) const;
};

#endif
