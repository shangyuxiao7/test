// ==================== BattleConfigLoader.cpp ====================
#include "BattleConfigLoader.h"
#include "BattleDataTables.h"
#include "../../foundation/ConfigLoader.h"
#include "../../foundation/JsonValue.h"

namespace {
    bool ParseSkills(const JsonValue& root) {
        if (root.GetType() != JsonType::Object) return false;
        const JsonValue& skillsArr = root["skills"];
        if (skillsArr.GetType() != JsonType::Array) return false;
        for (size_t i = 0; i < skillsArr.ArraySize(); ++i) {
            const JsonValue& sk = skillsArr[i];
            if (sk.GetType() != JsonType::Object) continue;
            SkillDef def;
            def.id             = sk["id"].AsString();
            def.name           = sk["name"].AsString();
            def.type           = sk["type"].AsString();
            def.qiCost         = sk["qiCost"].AsInt();
            def.damage         = sk["damage"].AsInt();
            def.healAmount     = sk["healAmount"].AsInt();
            def.effectName     = sk["effectName"].AsString();
            def.effectType     = sk["effectType"].AsString();
            def.effectValue    = sk["effectValue"].AsInt();
            def.effectDuration = sk["effectDuration"].AsInt();
            def.targetType     = sk["targetType"].AsString();
            BattleDataTables::Instance().RegisterSkill(def);
        }
        return true;
    }

    bool ParseEnemies(const JsonValue& root) {
        if (root.GetType() != JsonType::Object) return false;
        const JsonValue& enemiesArr = root["enemies"];
        if (enemiesArr.GetType() == JsonType::Array) {
            for (size_t i = 0; i < enemiesArr.ArraySize(); ++i) {
                const JsonValue& en = enemiesArr[i];
                if (en.GetType() != JsonType::Object) continue;
                EnemyDef def;
                def.id      = en["id"].AsString();
                def.name    = en["name"].AsString();
                def.hp      = en["hp"].AsInt();
                def.qi      = en["qi"].AsInt();
                def.speed   = en["speed"].AsInt();
                def.attack  = en["attack"].AsInt();
                def.defense = en["defense"].AsInt();
                const JsonValue& skillsArr = en["skills"];
                if (skillsArr.GetType() == JsonType::Array) {
                    for (size_t j = 0; j < skillsArr.ArraySize(); ++j) {
                        if (skillsArr[j].GetType() == JsonType::String)
                            def.skills.push_back(skillsArr[j].AsString());
                    }
                }
                BattleDataTables::Instance().RegisterEnemy(def);
            }
        }
        const JsonValue& groupsArr = root["groups"];
        if (groupsArr.GetType() == JsonType::Array) {
            for (size_t i = 0; i < groupsArr.ArraySize(); ++i) {
                const JsonValue& g = groupsArr[i];
                if (g.GetType() != JsonType::Object) continue;
                GroupDef def;
                def.id = g["id"].AsString();
                const JsonValue& membersArr = g["enemies"];
                if (membersArr.GetType() == JsonType::Array) {
                    for (size_t j = 0; j < membersArr.ArraySize(); ++j) {
                        if (membersArr[j].GetType() == JsonType::String)
                            def.enemies.push_back(membersArr[j].AsString());
                    }
                }
                BattleDataTables::Instance().RegisterGroup(def);
            }
        }
        return true;
    }
}

bool SkillConfigLoader::LoadFromFile(const std::string& filepath) {
    JsonValue root;
    if (!ConfigLoader::LoadFromFile(filepath, root)) return false;
    return ParseSkills(root);
}

bool SkillConfigLoader::LoadFromString(const std::string& content) {
    JsonValue root;
    if (!ConfigLoader::LoadFromString(content, root)) return false;
    return ParseSkills(root);
}

bool EnemyConfigLoader::LoadFromFile(const std::string& filepath) {
    JsonValue root;
    if (!ConfigLoader::LoadFromFile(filepath, root)) return false;
    return ParseEnemies(root);
}

bool EnemyConfigLoader::LoadFromString(const std::string& content) {
    JsonValue root;
    if (!ConfigLoader::LoadFromString(content, root)) return false;
    return ParseEnemies(root);
}
