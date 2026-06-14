#pragma once
#include <string>
#include "Types.h"
#include <map>
#include <vector>

// 物品配置定义（只读数据，从 JSON 配置表加载）
struct ItemDef {
    std::string id;
    std::string name;
    std::string description;
    ItemType type;
    int maxStack = 99;              // 最大堆叠数量
    int buyPrice = 0;
    int sellPrice = 0;

    // 装备特有属性（非装备时忽略）
    EquipmentSlot equipSlot = EquipmentSlot::Weapon;
    int bonusAttack = 0;
    int bonusDefense = 0;
    int bonusHp = 0;
    int bonusSpeed = 0;

    // 消耗品效果描述（由外部解析执行，背包系统不执行效果）
    std::string effectScript;       // 例如 "heal_hp:50" 或 "restore_qi:30"
};

// 配置表查询中心（全局单例，启动时加载所有 JSON 配置）
class ItemDataTables {
public:
    static ItemDataTables& Instance();
    // 查询物品定义，找不到返回 nullptr
    const ItemDef* GetItemDef(const std::string& itemId) const;

    // 获取所有已注册的物品定义
    std::vector<ItemDef> GetAllItemDefs() const;

    // 测试辅助：注册物品定义
    void RegisterItem(const ItemDef& def);
    void Clear();

private:
    std::map<std::string, ItemDef> items;
};