#pragma once
#include <string>
#include <unordered_map>
#include "RoomGraph.h"

struct MapDef {
    std::string id;
    std::string name;
    RoomGraph roomGraph;
};

class MapData {
public:
    void Clear();
    void RegisterMap(const MapDef& map);
    const MapDef* GetMap(const std::string& id) const;
    bool HasMap(const std::string& id) const;
    std::vector<std::string> GetAllMapIds() const;

private:
    std::unordered_map<std::string, MapDef> maps;
};
