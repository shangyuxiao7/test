// ==================== Action.h ====================
#pragma once
#include "ICombatant.h"

enum class TargetType {
    Enemy,
    Ally,
    Self,
    None
};

class Action {
protected:
    std::string name;
    int qiCost;

public:
    Action(const std::string& name, int qiCost) : name(name), qiCost(qiCost) {}
    virtual ~Action() = default;

    std::string GetName() const { return name; }
    int GetQiCost() const { return qiCost; }

    virtual void Execute(ICombatant* user, ICombatant* target) = 0;

    virtual bool IsAvailable(const ICombatant* user) const {
        return user->GetQi() >= qiCost;
    }

    virtual bool NeedTarget() const { return true; }

    virtual TargetType GetTargetType() const { return TargetType::Enemy; }
};

class NormalAttack : public Action {
public:
    NormalAttack() : Action("普通攻击", 0) {}

    void Execute(ICombatant* user, ICombatant* target) override {
        if (user && target) {
            user->Attack(target);
        }
    }
};

class RestoreQiAction : public Action {
    int restoreAmount;
public:
    RestoreQiAction(int amount = 20) : Action("回气", 0), restoreAmount(amount) {}

    bool NeedTarget() const override { return false; }
    TargetType GetTargetType() const override { return TargetType::None; }

    void Execute(ICombatant* user, ICombatant* /*target*/) override {
        if (user) {
            user->RestoreQi(restoreAmount);
        }
    }
};

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
