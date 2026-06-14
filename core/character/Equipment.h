#pragma once
#include "../../inventory/Types.h"
#include <string>

// 装备抽象基类 + 槽位枚举（枚举定义在 Types.h 中）
class EquipmentBase {
public:
    virtual ~EquipmentBase() = default;
    virtual std::string GetId() const = 0;
    virtual EquipmentSlot GetSlot() const = 0;
};

// 装备槽位工具函数
namespace EquipmentUtils {
    inline std::string SlotToString(EquipmentSlot slot) {
        switch (slot) {
            case EquipmentSlot::Weapon:    return "Weapon";
            case EquipmentSlot::Armor:     return "Armor";
            case EquipmentSlot::Shoes:     return "Shoes";
            case EquipmentSlot::Accessory: return "Accessory";
        }
        return "Unknown";
    }
}
