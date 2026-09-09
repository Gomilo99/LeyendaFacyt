#include "../lib/MapMetadata.hpp"
#include "../lib/json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

bool MapMetadata::load(const std::string& path) {
    std::ifstream file(path);
    if (!file) return false;
    json root;
    try {
        file >> root;
    } catch (const std::exception&) {
        return false;
    }
    section = root.value("section", section);
    levelCap = root.value("level_cap", levelCap);
    auto encounter = root.value("encounter", json::object());
    encounterBase = encounter.value("base", encounterBase);
    encounterMultiplier = encounter.value("multiplier", encounterMultiplier);
    encounterGrowthCap = encounter.value("growth_cap", encounterGrowthCap);
    encounterGraceSteps = encounter.value("grace_steps", encounterGraceSteps);
    tierColors.clear();
    auto tierColorConfig = root.value("tier_colors", json::object());
    for (auto it = tierColorConfig.begin(); it != tierColorConfig.end(); ++it)
        tierColors[std::stoi(it.key())] = it.value();
    healing.clear();
    auto healingConfig = root.value("healing", json::object());
    for (auto it = healingConfig.begin(); it != healingConfig.end(); ++it)
        if (!it.key().empty()) healing[it.key()[0]] = it.value();
    zones.clear();
    for (const auto& value : root.value("zones", json::array())) {
        ZoneMetadata z;
        z.id = value.value("id", "");
        const std::string tile = value.value("tile", "");
        if (!tile.empty()) z.tile = tile.front();
        z.terrain = value.value("terrain", "plain");
        z.terrainColor = value.value("color", 32);
        z.safe = value.value("safe", false);
        z.encounterMultiplier = value.value("encounter_multiplier", 1.0f);
        z.statMultiplier = value.value("stat_multiplier", 1.0f);
        z.xpMultiplier = value.value("xp_multiplier", 1.0f);
        z.bossId = value.value("boss_id", "");
        const std::string restoreTile = value.value("restore_tile", "");
        if (!restoreTile.empty()) z.restoreTile = restoreTile.front();
        for (const auto& e : value.value("enemies", json::array()))
            z.enemies.emplace_back(e.value("id", ""), e.value("weight", 1));
        zones.push_back(z);
    }
    return true;
}

const ZoneMetadata* MapMetadata::zoneAt(char tile) const {
    for (const auto& zone : zones) {
        if (zone.tile == tile)
            return &zone;
    }
    if (tile == 'P') {
        for (const auto& zone : zones)
            if (zone.safe) return &zone;
    }
    if (tile == 'B' || tile == 'K' ||
        tile == 'h' || tile == 'H' || tile == 'G') {
        for (const auto& zone : zones)
            if (!zone.safe) return &zone;
    }
    return nullptr;
}
