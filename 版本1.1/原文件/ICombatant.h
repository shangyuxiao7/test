// ==================== ICombatant.h ====================
#pragma once
#include <string>
#include "StatusEffect.h"

// 战斗单位抽象接口
class ICombatant {
public:
    virtual ~ICombatant() = default;

    // 基础信息
    virtual std::string GetName() const = 0;
    virtual bool IsAlive() const = 0;

    // 属性查询
    virtual int GetHp() const = 0;
    virtual int GetMaxHp() const = 0;
    virtual int GetAttack() const = 0;
    virtual int GetDefense() const = 0;

    // 新增：气力查询（供 Action::IsAvailable 使用）
    virtual int GetQi() const = 0;
    virtual int GetMaxQi() const = 0;

    // 战斗行为
    virtual void TakeDamage(int attackValue) = 0;
    virtual void Heal(int amount) = 0;
    virtual bool ConsumeQi(int cost) = 0;
    virtual void RestoreQi(int amount) = 0;

    // 普通攻击
    virtual void Attack(ICombatant* target) = 0;

    // 新增：施加状态效果（供 BuffSkill / CompositeSkill 使用）
    virtual void ApplyStatus(const StatusEffect& effect) = 0;
};