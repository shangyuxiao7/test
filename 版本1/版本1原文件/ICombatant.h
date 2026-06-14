// ==================== ICombatant.h ====================
#pragma once
#include <string>

// 战斗单位抽象接口 - 最底层，不依赖任何其他自定义类
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

    // 战斗行为
    virtual void TakeDamage(int attackValue) = 0;   // 受到攻击（参数为攻击方的攻击力）
    virtual void Heal(int amount) = 0;
    virtual bool ConsumeQi(int cost) = 0;
    virtual void RestoreQi(int amount) = 0;

    // 发起攻击（动作调用此方法，内部调用目标的 TakeDamage）
    virtual void Attack(ICombatant* target) = 0;
};