#include "MapData.h"

void MapData::Clear() { maps.clear(); }
void MapData::RegisterMap(const MapDef& map) { maps[map.id] = map; }
const MapDef* MapData::GetMap(const std::string& id) const {
    auto it = maps.find(id);
    return (it != maps.end()) ? &it->second : nullptr;
}
bool MapData::HasMap(const std::string& id) const { return maps.find(id) != maps.end(); }
std::vector<std::string> MapData::GetAllMapIds() const {
    std::vector<std::string> result;
    for (const auto& pair : maps) result.push_back(pair.first);
    return result;
}
