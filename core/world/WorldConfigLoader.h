#pragma once
#include <string>

class MapData;

class WorldConfigLoader {
public:
    static bool LoadFromFile(const std::string& filepath, MapData& out);
    static bool LoadFromString(const std::string& content, MapData& out);
};
