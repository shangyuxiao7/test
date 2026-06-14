#pragma once
#include <string>

// 物品配置加载器
// 职责：从 JSON 配置文件读取物品定义并注册到 DataTables
// 属于基础设施层，允许使用文件 I/O
class ItemConfigLoader {
public:
    // 从 JSON 文件加载物品配置到 DataTables
    static bool LoadFromFile(const std::string& filepath);
    // 从 JSON 字符串加载（用于测试或嵌入场景）
    static bool LoadFromString(const std::string& jsonContent);
};
