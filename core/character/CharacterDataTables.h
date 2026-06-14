#pragma once
#include "../inventory/Types.h"
#include <string>
#include <vector>
#include <unordered_map>

// 角色定义
struct CharacterDef {
    std::string id;
    std::string name;
    int baseHp = 0;
    int baseQi = 0;
    int baseSpeed = 0;
    int baseAttack = 0;
    int baseDefense = 0;
    std::vector<std::string> initialSkills;
    std::string growthCurveId;
};

// 装备定义
struct EquipmentDef {
    std::string id;
    std::string name;
    EquipmentSlot slot = EquipmentSlot::Weapon;
    int bonusAttack = 0;
    int bonusDefense = 0;
    int bonusHp = 0;
    int bonusQi = 0;
    int bonusSpeed = 0;
};

// 数据表查询（单模块验证桩，整合时由 core/foundation/ 提供真实版本）
class CharacterDataTables {
public:
    static CharacterDataTables& Instance() {
        static CharacterDataTables inst;
        return inst;
    }

    void RegisterCharacter(const CharacterDef& def) {
        characters[def.id] = def;
    }

    void RegisterEquipment(const EquipmentDef& def) {
        equipments[def.id] = def;
    }

    const CharacterDef* GetCharacterDef(const std::string& id) const {
        auto it = characters.find(id);
        return (it != characters.end()) ? &it->second : nullptr;
    }

    const EquipmentDef* GetEquipmentDef(const std::string& id) const {
        auto it = equipments.find(id);
        return (it != equipments.end()) ? &it->second : nullptr;
    }

    std::vector<CharacterDef> GetAllCharacterDefs() const {
        std::vector<CharacterDef> result;
        for (const auto& kv : characters) {
            result.push_back(kv.second);
        }
        return result;
    }

    std::vector<EquipmentDef> GetAllEquipmentDefs() const {
        std::vector<EquipmentDef> result;
        for (const auto& kv : equipments) {
            result.push_back(kv.second);
        }
        return result;
    }

    void ClearCharacters() {
        characters.clear();
    }

    void ClearEquipments() {
        equipments.clear();
    }

private:
    std::unordered_map<std::string, CharacterDef> characters;
    std::unordered_map<std::string, EquipmentDef> equipments;
};
