// ==================== SkillLibrary.h ====================
#pragma once
#include "Action.h"
#include "BattleDataTables.h"

class SkillLibrary {
public:
    static Action* Create(const std::string& name) {
        const SkillDef* def = BattleDataTables::Instance().GetSkillDef(name);
        if (!def) return nullptr;

        if (def->type == "DamageSkill") {
            return new DamageSkill(def->name, def->qiCost, def->damage);
        }
        else if (def->type == "HealSkill") {
            return new HealSkill(def->name, def->qiCost, def->healAmount);
        }
        else if (def->type == "BuffSkill") {
            EffectType et = ParseEffectType(def->effectType);
            StatusEffect effect(def->effectName, et, def->effectValue, def->effectDuration);
            TargetType tt = ParseTargetType(def->targetType);
            return new BuffSkill(def->name, def->qiCost, effect, tt);
        }
        else if (def->type == "CompositeSkill") {
            EffectType et = ParseEffectType(def->effectType);
            StatusEffect effect(def->effectName, et, def->effectValue, def->effectDuration);
            TargetType tt = ParseTargetType(def->targetType);
            return new CompositeSkill(def->name, def->qiCost, def->damage, effect, tt);
        }
        return nullptr;
    }

    static bool Exists(const std::string& name) {
        return BattleDataTables::Instance().GetSkillDef(name) != nullptr;
    }

private:
    static EffectType ParseEffectType(const std::string& s) {
        if (s == "AttackBuff") return EffectType::AttackBuff;
        if (s == "DefenseBuff") return EffectType::DefenseBuff;
        if (s == "AttackDebuff") return EffectType::AttackDebuff;
        if (s == "DefenseDebuff") return EffectType::DefenseDebuff;
        if (s == "HoT") return EffectType::HoT;
        if (s == "DoT") return EffectType::DoT;
        return EffectType::AttackBuff;
    }

    static TargetType ParseTargetType(const std::string& s) {
        if (s == "Enemy") return TargetType::Enemy;
        if (s == "Ally") return TargetType::Ally;
        if (s == "Self") return TargetType::Self;
        return TargetType::None;
    }
};
