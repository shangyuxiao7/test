#pragma once
#include <string>

// 物品类型
enum class ItemType {
    Consumable,     // 消耗品（药水、食物）
    Equipment,      // 装备（武器、防具、鞋子、宝物）
    Material,       // 材料（锻造用）
    Quest           // 任务道具
};

// 装备槽位
enum class EquipmentSlot {
    Weapon,         // 武器
    Armor,          // 防具
    Shoes,          // 鞋子
    Accessory       // 宝物/饰品
};

// 二维整数坐标（大地图用，背包系统不需要但可能在 Types.h 里）
struct Vector2Int {
    int x, y;
};
