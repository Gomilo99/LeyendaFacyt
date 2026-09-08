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
    terrainStyles.clear();
    auto styles = root.value("terrain_styles", json::object());
    for (auto it = styles.begin(); it != styles.end(); ++it) {
        terrainStyles[it.key()] = {it.value().value("color", 32),
                                   it.value().value("style", ".")};
    }
    healing.clear();
    auto healingConfig = root.value("healing", json::object());
    for (auto it = healingConfig.begin(); it != healingConfig.end(); ++it)
        if (!it.key().empty()) healing[it.key()[0]] = it.value();
    zones.clear();
    for (const auto& value : root.value("zones", json::array())) {
        ZoneMetadata z;
        z.id = value.value("id", "");
        z.x = value.value("x", 0); z.y = value.value("y", 0);
        z.width = value.value("width", 0); z.height = value.value("height", 0);
        z.terrain = value.value("terrain", "plain");
        auto style = terrainStyles.find(z.terrain);
        if (style != terrainStyles.end()) {
            z.terrainColor = style->second.first;
            z.terrainStyle = style->second.second;
        }
        z.safe = value.value("safe", false);
        z.encounterMultiplier = value.value("encounter_multiplier", 1.0f);
        z.statMultiplier = value.value("stat_multiplier", 1.0f);
        z.xpMultiplier = value.value("xp_multiplier", 1.0f);
        z.bossId = value.value("boss_id", "");
        for (const auto& e : value.value("enemies", json::array()))
            z.enemies.emplace_back(e.value("id", ""), e.value("weight", 1));
        zones.push_back(z);
    }
    return true;
}

const ZoneMetadata* MapMetadata::zoneAt(int x, int y) const {
    const ZoneMetadata* result = nullptr;
    for (const auto& zone : zones)
        if (zone.contains(x, y) &&
            (!result || zone.width * zone.height < result->width * result->height))
            result = &zone;
    return result;
}
