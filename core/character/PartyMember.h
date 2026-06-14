#pragma once
#include "../../foundation/ISerializable.h"
#include "../inventory/Types.h"
#include <string>
#include <vector>
#include <map>

// 战斗初始化数据（导出给战斗系统的纯数据结构）
struct CombatantInitData {
    std::string characterId;   // 配置表中的角色唯一ID
    std::string name;
    int level = 1;
    int hp = 0, maxHp = 0;
    int qi = 0, maxQi = 0;
    int speed = 0;
    int attack = 0;
    int defense = 0;
    std::vector<std::string> skillIds;
};

// 角色持久数据（战斗外角色养成）
class PartyMember : public ISerializable {
public:
    PartyMember(const std::string& characterId, int level = 1);

    // 养成操作
    void GainExp(int amount);
    void Equip(EquipmentSlot slot, const std::string& equipmentId);
    void Unequip(EquipmentSlot slot);
    void SetActive(bool active);
    void SetDead(bool dead);
    void LearnSkill(const std::string& skillId);

    // 导出/同步
    CombatantInitData ExportForBattle() const;
    void SyncAfterBattle(int remainingHp, int remainingQi, bool isDead);

    // 查询接口
    int GetLevel() const;
    int GetCurrentHp() const;
    int GetMaxHp() const;
    int GetCurrentQi() const;
    int GetMaxQi() const;
    int GetAttack() const;      // 含装备加成
    int GetDefense() const;     // 含装备加成
    int GetSpeed() const;       // 含装备加成
    bool IsActive() const;
    bool IsDead() const;
    std::vector<std::string> GetSkillIds() const;
    std::string GetEquippedItem(EquipmentSlot slot) const;
    std::string GetCharacterId() const;
    int GetCurrentExp() const;
    int GetExpToNextLevel() const;

    // 序列化
    void Serialize(std::string& out) const override;
    void Deserialize(const std::string& in) override;

private:
    std::string characterId;
    int level = 1;
    int currentHp = 0;
    int currentQi = 0;
    int currentExp = 0;
    bool active = true;
    bool dead = false;
    std::vector<std::string> skillIds;
    std::map<EquipmentSlot, std::string> equipments; // 槽位 -> 装备ID

    void RecalculateStats();

    // 基础属性（不含装备）
    int GetBaseMaxHp() const;
    int GetBaseMaxQi() const;
    int GetBaseAttack() const;
    int GetBaseDefense() const;
    int GetBaseSpeed() const;

    // 装备加成
    int GetEquipmentBonusHp() const;
    int GetEquipmentBonusQi() const;
    int GetEquipmentBonusAttack() const;
    int GetEquipmentBonusDefense() const;
    int GetEquipmentBonusSpeed() const;
};
