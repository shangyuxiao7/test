#pragma once
#include "Types.h"
#include "ItemDataTables.h"
#include <string>

// 背包中的物品堆叠
struct ItemStack {
    std::string itemId;     // 配置表中的物品ID
    int count = 0;          // 当前数量

    // 查询属性（委托到 DataTables，不在自身存储冗余数据）
    std::string GetName() const {
        const ItemDef* def = ItemDataTables::Instance().GetItemDef(itemId);
        return def ? def->name : "未知物品";
    }

    ItemType GetType() const {
        const ItemDef* def = ItemDataTables::Instance().GetItemDef(itemId);
        return def ? def->type : ItemType::Consumable;
    }

    int GetMaxStack() const {
        const ItemDef* def = ItemDataTables::Instance().GetItemDef(itemId);
        return def ? def->maxStack : 99;
    }

    bool IsEquipment() const {
        const ItemDef* def = ItemDataTables::Instance().GetItemDef(itemId);
        return def && def->type == ItemType::Equipment;
    }
};
