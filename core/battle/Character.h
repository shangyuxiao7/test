// ==================== Character.h ====================
#pragma once
#include "ICombatant.h"
#include "GameEvent.h"
#include "Attributes.h"
#include "StatusEffect.h"
#include <vector>
#include <algorithm>

class Action;

class Character : public ICombatant {
protected:
    std::string name;
    Attributes attr;
    Faction faction;
    bool alive;
    std::vector<Action*> actions;
    std::vector<Action*> skills;
    DisplayCallback displayCallback;

    std::vector<StatusEffect> activeEffects;
    int attackMod = 0;
    int defenseMod = 0;
    int speedMod = 0;

public:
    Character() : name("Unknown"), faction(Faction::Player), alive(true) {}
    explicit Character(const std::string& name) : name(name), faction(Faction::Player), alive(true) {}
    Character(const std::string& name, const Attributes& attr, Faction faction)
        : name(name), attr(attr), faction(faction), alive(true) {
    }

    virtual ~Character();

    void SetDisplayCallback(DisplayCallback callback) {
        displayCallback = callback;
    }

    void Notify(BattleEventType type, int value = 0, const std::string& targetName = "", const std::string& msg = "") {
        if (displayCallback) {
            displayCallback({
                type, name, targetName, value,
                attr.hp, attr.qi, attr.maxHp,
                attr.attack + attackMod, attr.defense + defenseMod, attr.speed + speedMod,
                msg
                });
        }
    }

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
            Notify(BattleEventType::Died, damage);
        }
        else {
            Notify(BattleEventType::DamageTaken, damage);
        }
    }

    void Heal(int amount) override {
        if (!alive || amount <= 0) return;
        int actualHeal = std::min(amount, attr.maxHp - attr.hp);
        attr.hp += actualHeal;
        Notify(BattleEventType::Healed, actualHeal);
    }

    bool ConsumeQi(int cost) override {
        if (attr.qi < cost) {
            Notify(BattleEventType::QiConsumed, 0, "", "气力不足");
            return false;
        }
        attr.qi -= cost;
        Notify(BattleEventType::QiConsumed, cost);
        return true;
    }

    void RestoreQi(int amount) override {
        attr.qi = std::min(attr.qi + amount, attr.maxQi);
        Notify(BattleEventType::QiRestored, amount);
    }

    void Attack(ICombatant* target) override {
        if (!alive || !target) return;
        Notify(BattleEventType::Attack, attr.attack + attackMod, target->GetName());
        target->TakeDamage(attr.attack + attackMod);
    }

    void ApplyStatus(const StatusEffect& effect) override {
        if (!alive) return;
        for (auto& e : activeEffects) {
            if (e.name == effect.name) {
                e.duration = effect.maxDuration;
                Notify(BattleEventType::BuffApplied, effect.value, "", effect.name + " 效果刷新");
                RecalculateStats();
                return;
            }
        }
        activeEffects.push_back(effect);
        Notify(BattleEventType::BuffApplied, effect.value, "", effect.name + " 施加成功");
        RecalculateStats();
    }

    void UpdateEffects() {
        if (activeEffects.empty()) return;

        for (auto it = activeEffects.begin(); it != activeEffects.end(); ) {
            if (it->type == EffectType::DoT) {
                attr.hp -= it->value;
                if (attr.hp < 0) attr.hp = 0;
                Notify(BattleEventType::DotTick, it->value, "", it->name + " 持续伤害");
                if (attr.hp <= 0 && alive) {
                    alive = false;
                    Notify(BattleEventType::Died, 0);
                }
            }
            else if (it->type == EffectType::HoT) {
                int actualHeal = std::min(it->value, attr.maxHp - attr.hp);
                attr.hp += actualHeal;
                Notify(BattleEventType::DotTick, actualHeal, "", it->name + " 持续治疗");
            }

            it->duration--;
            if (it->IsExpired()) {
                Notify(BattleEventType::BuffExpired, 0, "", it->name + " 效果消失");
                it = activeEffects.erase(it);
            }
            else {
                ++it;
            }
        }
        RecalculateStats();
    }

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
            Notify(BattleEventType::StatChanged, 0, "", "属性发生变化");
        }
    }

    void PerformAttack(Character* target) {
        Attack(target);
    }

    int GetSpeed() const { return attr.speed + speedMod; }
    Faction GetFaction() const { return faction; }
    const Attributes& GetAttributes() const { return attr; }
    const std::vector<StatusEffect>& GetActiveEffects() const { return activeEffects; }

    void AddAction(Action* action);
    std::vector<Action*> GetActions() const { return actions; }

    void AddSkill(Action* action);
    std::vector<Action*> GetSkills() const { return skills; }

    virtual void TakeTurn() = 0;
    virtual Character* SelectTarget(std::vector<Character*>& enemies) = 0;
    virtual Action* SelectAction() = 0;
};
