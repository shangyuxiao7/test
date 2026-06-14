// ==================== Character.h ====================
#pragma once
#include "ICombatant.h"
#include "GameEvent.h"
#include "Attributes.h"
#include <vector>
#include <algorithm>

// 前置声明 - Action 类
class Action;

// 角色基类 - 实现 ICombatant 接口
class Character : public ICombatant {
protected:
    std::string name;
    Attributes attr;
    Faction faction;
    bool alive;
    std::vector<Action*> actions;
    DisplayCallback displayCallback;

public:
    Character() : name("无名"), faction(Faction::Player), alive(true) {}
    explicit Character(const std::string& name) : name(name), faction(Faction::Player), alive(true) {}
    Character(const std::string& name, const Attributes& attr, Faction faction)
        : name(name), attr(attr), faction(faction), alive(true) {}
    virtual ~Character() = default;

    void SetDisplayCallback(DisplayCallback callback) {
        displayCallback = callback;
    }

    void Notify(EventType type, int value = 0, const std::string& targetName = "", const std::string& msg = "") {
        if (displayCallback) {
            displayCallback({
                type, name, targetName, value,
                attr.hp, attr.qi, attr.maxHp,
                attr.attack, attr.defense, attr.speed,
                msg
                });
        }
    }

    // ----- 实现 ICombatant 接口 -----
    std::string GetName() const override { return name; }
    bool IsAlive() const override { return alive; }
    int GetHp() const override { return attr.hp; }
    int GetMaxHp() const override { return attr.maxHp; }
    int GetAttack() const override { return attr.attack; }
    int GetDefense() const override { return attr.defense; }

    void TakeDamage(int attackValue) override {
        if (!alive) return;
        int damage = std::max(attackValue - attr.defense, 1);
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
        Notify(EventType::Attack, attr.attack, target->GetName());
        target->TakeDamage(attr.attack);
    }
    // ----- 接口实现结束 -----

    // 保留原有便捷方法（内部调用接口方法）
    void PerformAttack(Character* target) {
        Attack(target);
    }

    // Getter 补充（部分已在接口中，这里提供额外的）
    int GetQi() const { return attr.qi; }
    int GetMaxQi() const { return attr.maxQi; }
    int GetSpeed() const { return attr.speed; }
    Faction GetFaction() const { return faction; }
    const Attributes& GetAttributes() const { return attr; }

    // 动作管理
    void AddAction(Action* action);   // 实现在 Character.cpp 中
    std::vector<Action*> GetActions() const { return actions; }

    // 纯虚接口（子类必须实现）
    virtual void TakeTurn() = 0;
    virtual Character* SelectTarget(std::vector<Character*>& enemies) = 0;
    virtual Action* SelectAction() = 0;
};