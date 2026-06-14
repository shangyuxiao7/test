#include "ItemDataTables.h"

ItemDataTables& ItemDataTables::Instance() {
    static ItemDataTables instance;
    return instance;
}

const ItemDef* ItemDataTables::GetItemDef(const std::string& itemId) const {
    auto it = items.find(itemId);
    if (it != items.end()) return &(it->second);
    return nullptr;
}

std::vector<ItemDef> ItemDataTables::GetAllItemDefs() const {
    std::vector<ItemDef> result;
    for (const auto& pair : items) {
        result.push_back(pair.second);
    }
    return result;
}

void ItemDataTables::RegisterItem(const ItemDef& def) {
    items[def.id] = def;
}

void ItemDataTables::Clear() {
    items.clear();
}