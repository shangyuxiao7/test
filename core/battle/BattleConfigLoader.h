// ==================== BattleConfigLoader.h ====================
#pragma once
#include <string>

class SkillConfigLoader {
public:
    static bool LoadFromFile(const std::string& filepath);
    static bool LoadFromString(const std::string& content);
};

class EnemyConfigLoader {
public:
    static bool LoadFromFile(const std::string& filepath);
    static bool LoadFromString(const std::string& content);
};
