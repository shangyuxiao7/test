// ==================== EnemyFactory.h ====================
#pragma once
#include "Character.h"
#include "AICharacter.h"
#include "Action.h"
#include "SkillLibrary.h"
#include "BattleDataTables.h"
#include <vector>

class EnemyFactory {
private:
    static Character* CreateFromDef(const std::string& id) {
        const EnemyDef* def = BattleDataTables::Instance().GetEnemyDef(id);
        if (!def) return nullptr;
        Character* e = new AICharacter(def->name,
            Attributes(def->hp, def->qi, def->speed, def->attack, def->defense),
            Faction::Enemy);
        e->AddAction(new NormalAttack());
        for (const auto& skillId : def->skills) {
            Action* skill = SkillLibrary::Create(skillId);
            if (skill) e->AddSkill(skill);
        }
        return e;
    }

public:
    static std::vector<Character*> CreateGroup(const std::string& name) {
        const GroupDef* def = BattleDataTables::Instance().GetGroupDef(name);
        if (!def) return std::vector<Character*>();
        std::vector<Character*> result;
        for (const auto& enemyId : def->enemies) {
            Character* e = CreateFromDef(enemyId);
            if (e) result.push_back(e);
        }
        return result;
    }

    static bool GroupExists(const std::string& name) {
        return BattleDataTables::Instance().GetGroupDef(name) != nullptr;
    }
};
