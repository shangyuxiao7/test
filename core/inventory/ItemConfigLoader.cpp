#include "ItemConfigLoader.h"
#include "ItemDataTables.h"
#include "Types.h"
#include "../../foundation/ConfigLoader.h"
#include "../../foundation/JsonValue.h"

namespace {
    ItemType ParseItemType(const std::string& str) {
        if (str == "Consumable") return ItemType::Consumable;
        if (str == "Equipment")  return ItemType::Equipment;
        if (str == "Material")   return ItemType::Material;
        if (str == "Quest")      return ItemType::Quest;
        return ItemType::Consumable;
    }

    EquipmentSlot ParseEquipSlot(const std::string& str) {
        if (str == "Weapon")    return EquipmentSlot::Weapon;
        if (str == "Armor")     return EquipmentSlot::Armor;
        if (str == "Shoes")     return EquipmentSlot::Shoes;
        if (str == "Accessory") return EquipmentSlot::Accessory;
        return EquipmentSlot::Weapon;
    }

    bool ParseItems(const JsonValue& root) {
        ItemDataTables::Instance().Clear();
        if (root.GetType() != JsonType::Object) return false;
        const JsonValue& itemsArr = root["items"];
        if (itemsArr.GetType() != JsonType::Array) return false;
        for (size_t i = 0; i < itemsArr.ArraySize(); ++i) {
            const JsonValue& node = itemsArr[i];
            if (node.GetType() != JsonType::Object) continue;
            ItemDef def;
            def.id           = node["id"].AsString();
            def.name         = node["name"].AsString();
            def.description  = node["description"].AsString();
            def.type         = ParseItemType(node["type"].AsString());
            def.maxStack     = node["maxStack"].AsInt();
            def.buyPrice     = node["buyPrice"].AsInt();
            def.sellPrice    = node["sellPrice"].AsInt();
            def.equipSlot    = ParseEquipSlot(node["equipSlot"].AsString());
            def.bonusAttack  = node["bonusAttack"].AsInt();
            def.bonusDefense = node["bonusDefense"].AsInt();
            def.bonusHp      = node["bonusHp"].AsInt();
            def.bonusSpeed   = node["bonusSpeed"].AsInt();
            def.effectScript = node["effectScript"].AsString();
            if (!def.id.empty()) {
                ItemDataTables::Instance().RegisterItem(def);
            }
        }
        return true;
    }
}

bool ItemConfigLoader::LoadFromFile(const std::string& filepath) {
    JsonValue root;
    if (!ConfigLoader::LoadFromFile(filepath, root)) return false;
    return ParseItems(root);
}

bool ItemConfigLoader::LoadFromString(const std::string& content) {
    JsonValue root;
    if (!ConfigLoader::LoadFromString(content, root)) return false;
    return ParseItems(root);
}
