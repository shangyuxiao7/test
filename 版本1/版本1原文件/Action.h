// ==================== Action.h ====================
#pragma once
#include "ICombatant.h"

// Action 基类 - 现在只依赖 ICombatant 接口
class Action {
protected:
    std::string name;
    int qiCost;

public:
    Action(const std::string& name, int qiCost) : name(name), qiCost(qiCost) {}
    virtual ~Action() = default;

    std::string GetName() const { return name; }
    int GetQiCost() const { return qiCost; }

    // 执行操作，参数均为接口指针
    virtual void Execute(ICombatant* user, ICombatant* target) = 0;
};

// 普通攻击实现
class NormalAttack : public Action {
public:
    NormalAttack() : Action("普通攻击", 0) {}

    void Execute(ICombatant* user, ICombatant* target) override {
        if (user && target) {
            user->Attack(target);   // 通过接口调用攻击
        }
    }
};