// ==================== Action.h ====================
#pragma once
#include "ICombatant.h"

// 技能目标类型
enum class TargetType {
    Enemy,   // 敌方目标
    Ally,    // 友方目标
    Self,    // 自身
    None     // 无需目标
};

// Action 抽象基类
class Action {
protected:
    std::string name;
    int qiCost;

public:
    Action(const std::string& name, int qiCost) : name(name), qiCost(qiCost) {}
    virtual ~Action() = default;

    std::string GetName() const { return name; }
    int GetQiCost() const { return qiCost; }

    // 执行行为
    virtual void Execute(ICombatant* user, ICombatant* target) = 0;

    // 专门判断当前技能是否可用（默认检查气力，派生类可重写以支持复杂条件）
    virtual bool IsAvailable(const ICombatant* user) const {
        return user->GetQi() >= qiCost;
    }

    // 是否需要选择目标
    virtual bool NeedTarget() const { return true; }

    // 目标类型（用于 UI 自动切换目标队伍）
    virtual TargetType GetTargetType() const { return TargetType::Enemy; }
};

// 普通攻击
class NormalAttack : public Action {
public:
    NormalAttack() : Action("普通攻击", 0) {}

    void Execute(ICombatant* user, ICombatant* target) override {
        if (user && target) {
            user->Attack(target);
        }
    }
};

// 回蓝行为（无需目标）
class RestoreQiAction : public Action {
    int restoreAmount;
public:
    RestoreQiAction(int amount = 20) : Action("回蓝", 0), restoreAmount(amount) {}

    bool NeedTarget() const override { return false; }
    TargetType GetTargetType() const override { return TargetType::None; }

    void Execute(ICombatant* user, ICombatant* /*target*/) override {
        if (user) {
            user->RestoreQi(restoreAmount);
        }
    }
};

// 伤害技能
class DamageSkill : public Action {
    int damage;
public:
    DamageSkill(const std::string& name, int qiCost, int damage)
        : Action(name, qiCost), damage(damage) {
    }

    void Execute(ICombatant* user, ICombatant* target) override {
        if (!user || !target) return;
        if (!user->ConsumeQi(qiCost)) return;
        target->TakeDamage(damage);
    }
};

// 治疗技能（目标为友方）
class HealSkill : public Action {
    int healAmount;
public:
    HealSkill(const std::string& name, int qiCost, int healAmount)
        : Action(name, qiCost), healAmount(healAmount) {
    }

    TargetType GetTargetType() const override { return TargetType::Ally; }

    void Execute(ICombatant* user, ICombatant* target) override {
        if (!user || !target) return;
        if (!user->ConsumeQi(qiCost)) return;
        target->Heal(healAmount);
    }
};

// Buff / Debuff 技能
class BuffSkill : public Action {
    StatusEffect effect;
    TargetType targetType;
public:
    BuffSkill(const std::string& name, int qiCost, const StatusEffect& effect, TargetType t = TargetType::Ally)
        : Action(name, qiCost), effect(effect), targetType(t) {
    }

    TargetType GetTargetType() const override { return targetType; }

    void Execute(ICombatant* user, ICombatant* target) override {
        if (!user) return;
        if (!user->ConsumeQi(qiCost)) return;
        if (target) {
            target->ApplyStatus(effect);
        }
    }
};

// 组合技能：伤害 + 施加 buff
class CompositeSkill : public Action {
    int damage;
    StatusEffect effect;
    TargetType targetType;
public:
    CompositeSkill(const std::string& name, int qiCost, int damage, const StatusEffect& effect, TargetType t = TargetType::Enemy)
        : Action(name, qiCost), damage(damage), effect(effect), targetType(t) {
    }

    TargetType GetTargetType() const override { return targetType; }

    void Execute(ICombatant* user, ICombatant* target) override {
        if (!user || !target) return;
        if (!user->ConsumeQi(qiCost)) return;
        target->TakeDamage(damage);
        target->ApplyStatus(effect);
    }
};