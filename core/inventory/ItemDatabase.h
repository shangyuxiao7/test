#pragma once
#include "Types.h"
#include "ItemDataTables.h"
#include <string>
#include <vector>

// 物品数据库查询封装
// 职责：提供物品相关的便捷查询，不存储运行时状态
class ItemDatabase {
public:
    // 获取物品定义
    static const ItemDef* GetDef(const std::string& itemId);

    // 获取物品名称（找不到返回 "未知物品"）
    static std::string GetName(const std::string& itemId);

    // 判断是否为装备
    static bool IsEquipment(const std::string& itemId);

    // 判断是否为消耗品
    static bool IsConsumable(const std::string& itemId);

    // 获取装备槽位（非装备返回默认值 Weapon）
    static EquipmentSlot GetEquipSlot(const std::string& itemId);
};
