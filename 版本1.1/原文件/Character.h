// ==================== Character.h ====================
#pragma once
#include "ICombatant.h"
#include "GameEvent.h"
#include "Attributes.h"
#include "StatusEffect.h"
#include <vector>
#include <algorithm>

// 前向声明
class Action;

// 角色抽象基类
class Character : public ICombatant {
protected:
    std::string name;
    Attributes attr;
    Faction faction;
    bool alive;
    std::vector<Action*> actions;
    std::vector<Action*> skills;
    DisplayCallback displayCallback;

    // 状态效果系统
    std::vector<StatusEffect> activeEffects;
    int attackMod = 0;
    int defenseMod = 0;
    int speedMod = 0;

public:
    Character() : name("未命名"), faction(Faction::Player), alive(true) {}
    explicit Character(const std::string& name) : name(name), faction(Faction::Player), alive(true) {}
    Character(const std::string& name, const Attributes& attr, Faction faction)
        : name(name), attr(attr), faction(faction), alive(true) {
    }

    virtual ~Character();

    void SetDisplayCallback(DisplayCallback callback) {
        displayCallback = callback;
    }

    void Notify(EventType type, int value = 0, const std::string& targetName = "", const std::string& msg = "") {
        if (displayCallback) {
            displayCallback({
                type, name, targetName, value,
                attr.hp, attr.qi, attr.maxHp,
                attr.attack + attackMod, attr.defense + defenseMod, attr.speed + speedMod,
                msg
                });
        }
    }

    // ----- 实现 ICombatant 接口 -----
    std::string GetName() const override { return name; }
    bool IsAlive() const override { return alive; }
    int GetHp() const override { return attr.hp; }
    int GetMaxHp() const override { return attr.maxHp; }
    int GetAttack() const override { return attr.attack + attackMod; }
    int GetDefense() const override { return attr.defense + defenseMod; }
    int GetQi() const override { return attr.qi; }
    int GetMaxQi() const override { return attr.maxQi; }

    void TakeDamage(int attackValue) override {
        if (!alive) return;
        int damage = std::max(attackValue - (attr.defense + defenseMod), 1);
        attr.hp -= damage;
        if (attr.hp <= 0) {
            attr.hp = 0;
            alive = false;
            Notify(EventType::Died, damage);
        }
        else {
            Notify(EventType::DamageTaken, damage);
        }
    }

    void Heal(int amount) override {
        if (!alive || amount <= 0) return;
        int actualHeal = std::min(amount, attr.maxHp - attr.hp);
        attr.hp += actualHeal;
        Notify(EventType::Healed, actualHeal);
    }

    bool ConsumeQi(int cost) override {
        if (attr.qi < cost) {
            Notify(EventType::QiConsumed, 0, "", "气力不足");
            return false;
        }
        attr.qi -= cost;
        Notify(EventType::QiConsumed, cost);
        return true;
    }

    void RestoreQi(int amount) override {
        attr.qi = std::min(attr.qi + amount, attr.maxQi);
        Notify(EventType::QiRestored, amount);
    }

    void Attack(ICombatant* target) override {
        if (!alive || !target) return;
        Notify(EventType::Attack, attr.attack + attackMod, target->GetName());
        target->TakeDamage(attr.attack + attackMod);
    }
    // ----- 接口实现结束 -----

    // 施加状态效果（实现 ICombatant::ApplyStatus）
    void ApplyStatus(const StatusEffect& effect) override {
        if (!alive) return;
        // 同名效果刷新回合
        for (auto& e : activeEffects) {
            if (e.name == effect.name) {
                e.duration = effect.maxDuration;
                Notify(EventType::BuffApplied, effect.value, "", effect.name + " 效果刷新");
                RecalculateStats();
                return;
            }
        }
        activeEffects.push_back(effect);
        Notify(EventType::BuffApplied, effect.value, "", effect.name + " 施加成功");
        RecalculateStats();
    }

    // 每回合结算状态效果（DoT/HoT + 回合衰减）
    void UpdateEffects() {
        if (activeEffects.empty()) return;

        for (auto it = activeEffects.begin(); it != activeEffects.end(); ) {
            if (it->type == EffectType::DoT) {
                attr.hp -= it->value;
                if (attr.hp < 0) attr.hp = 0;
                Notify(EventType::DotTick, it->value, "", it->name + " 造成持续伤害");
                if (attr.hp <= 0 && alive) {
                    alive = false;
                    Notify(EventType::Died, 0);
                }
            }
            else if (it->type == EffectType::HoT) {
                int actualHeal = std::min(it->value, attr.maxHp - attr.hp);
                attr.hp += actualHeal;
                Notify(EventType::DotTick, actualHeal, "", it->name + " 恢复生命");
            }

            it->duration--;
            if (it->IsExpired()) {
                Notify(EventType::BuffExpired, 0, "", it->name + " 效果消失");
                it = activeEffects.erase(it);
            }
            else {
                ++it;
            }
        }
        RecalculateStats();
    }

    // 重新计算临时属性修正
    void RecalculateStats() {
        attackMod = 0;
        defenseMod = 0;
        speedMod = 0;
        for (const auto& effect : activeEffects) {
            switch (effect.type) {
            case EffectType::AttackBuff: attackMod += effect.value; break;
            case EffectType::DefenseBuff: defenseMod += effect.value; break;
            case EffectType::AttackDebuff: attackMod -= effect.value; break;
            case EffectType::DefenseDebuff: defenseMod -= effect.value; break;
            default: break;
            }
        }
        if (attackMod != 0 || defenseMod != 0 || speedMod != 0) {
            Notify(EventType::StatChanged, 0, "", "状态效果使属性发生变化");
        }
    }

    // 兼容旧版的 PerformAttack
    void PerformAttack(Character* target) {
        Attack(target);
    }

    // Getters
    int GetSpeed() const { return attr.speed + speedMod; }
    Faction GetFaction() const { return faction; }
    const Attributes& GetAttributes() const { return attr; }
    const std::vector<StatusEffect>& GetActiveEffects() const { return activeEffects; }

    // 行为管理
    void AddAction(Action* action);
    std::vector<Action*> GetActions() const { return actions; }

    // 技能管理
    void AddSkill(Action* action);
    std::vector<Action*> GetSkills() const { return skills; }

    // 子类必须实现的回合接口
    virtual void TakeTurn() = 0;
    virtual Character* SelectTarget(std::vector<Character*>& enemies) = 0;
    virtual Action* SelectAction() = 0;
};