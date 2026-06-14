// ==================== DataTables.cpp ====================
#include "BattleDataTables.h"

void BattleDataTables::Clear() {
    skills.clear();
    enemies.clear();
    groups.clear();
}

void BattleDataTables::RegisterSkill(const SkillDef& def) { skills[def.id] = def; }
void BattleDataTables::RegisterEnemy(const EnemyDef& def) { enemies[def.id] = def; }
void BattleDataTables::RegisterGroup(const GroupDef& def) { groups[def.id] = def; }

const SkillDef* BattleDataTables::GetSkillDef(const std::string& id) const {
    auto it = skills.find(id);
    return (it != skills.end()) ? &it->second : nullptr;
}

const EnemyDef* BattleDataTables::GetEnemyDef(const std::string& id) const {
    auto it = enemies.find(id);
    return (it != enemies.end()) ? &it->second : nullptr;
}

const GroupDef* BattleDataTables::GetGroupDef(const std::string& id) const {
    auto it = groups.find(id);
    return (it != groups.end()) ? &it->second : nullptr;
}

std::vector<SkillDef> BattleDataTables::GetAllSkillDefs() const {
    std::vector<SkillDef> result;
    for (const auto& kv : skills) result.push_back(kv.second);
    return result;
}

std::vector<EnemyDef> BattleDataTables::GetAllEnemyDefs() const {
    std::vector<EnemyDef> result;
    for (const auto& kv : enemies) result.push_back(kv.second);
    return result;
}

std::vector<GroupDef> BattleDataTables::GetAllGroupDefs() const {
    std::vector<GroupDef> result;
    for (const auto& kv : groups) result.push_back(kv.second);
    return result;
}
