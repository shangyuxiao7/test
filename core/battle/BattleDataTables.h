// ==================== DataTables.h ====================
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct SkillDef {
    std::string id;
    std::string name;
    std::string type;
    int qiCost = 0;
    int damage = 0;
    int healAmount = 0;
    std::string effectName;
    std::string effectType;
    int effectValue = 0;
    int effectDuration = 0;
    std::string targetType;
};

struct EnemyDef {
    std::string id;
    std::string name;
    int hp = 0;
    int qi = 0;
    int speed = 0;
    int attack = 0;
    int defense = 0;
    std::vector<std::string> skills;
};

struct GroupDef {
    std::string id;
    std::vector<std::string> enemies;
};

class BattleDataTables {
public:
    static BattleDataTables& Instance() {
        static BattleDataTables inst;
        return inst;
    }

    void Clear();
    void RegisterSkill(const SkillDef& def);
    void RegisterEnemy(const EnemyDef& def);
    void RegisterGroup(const GroupDef& def);

    const SkillDef* GetSkillDef(const std::string& id) const;
    const EnemyDef* GetEnemyDef(const std::string& id) const;
    const GroupDef* GetGroupDef(const std::string& id) const;

    std::vector<SkillDef> GetAllSkillDefs() const;
    std::vector<EnemyDef> GetAllEnemyDefs() const;
    std::vector<GroupDef> GetAllGroupDefs() const;

private:
    std::unordered_map<std::string, SkillDef> skills;
    std::unordered_map<std::string, EnemyDef> enemies;
    std::unordered_map<std::string, GroupDef> groups;
};
