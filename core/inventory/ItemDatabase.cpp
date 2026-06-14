#include "ItemDatabase.h"
#include "ItemDataTables.h"

const ItemDef* ItemDatabase::GetDef(const std::string& itemId) {
    return ItemDataTables::Instance().GetItemDef(itemId);
}

std::string ItemDatabase::GetName(const std::string& itemId) {
    const ItemDef* def = GetDef(itemId);
    return def ? def->name : "未知物品";
}

bool ItemDatabase::IsEquipment(const std::string& itemId) {
    const ItemDef* def = GetDef(itemId);
    return def && def->type == ItemType::Equipment;
}

bool ItemDatabase::IsConsumable(const std::string& itemId) {
    const ItemDef* def = GetDef(itemId);
    return def && def->type == ItemType::Consumable;
}

EquipmentSlot ItemDatabase::GetEquipSlot(const std::string& itemId) {
    const ItemDef* def = GetDef(itemId);
    return def ? def->equipSlot : EquipmentSlot::Weapon;
}
