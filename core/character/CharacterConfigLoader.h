#pragma once
#include <string>

// 配置加载器：从 JSON 文件读取角色/装备/曲线定义，注入全局配置中心
class CharacterConfigLoader {
public:
    static bool LoadCharactersFromFile(const std::string& filepath);
    static bool LoadEquipmentsFromFile(const std::string& filepath);
    static bool LoadCurvesFromFile(const std::string& filepath);

    // 统一入口：从指定目录加载角色、装备、曲线三个配置文件
    // dir 为目录路径，内部会拼接 dir/characters.json、equipments.json、curves.json
    static bool LoadAllFromDirectory(const std::string& dir);
};
