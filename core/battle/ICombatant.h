// ==================== ICombatant.h ====================
#pragma once
#include <string>
#include "StatusEffect.h"

class ICombatant {
public:
    virtual ~ICombatant() = default;

    virtual std::string GetName() const = 0;
    virtual bool IsAlive() const = 0;

    virtual int GetHp() const = 0;
    virtual int GetMaxHp() const = 0;
    virtual int GetAttack() const = 0;
    virtual int GetDefense() const = 0;

    virtual int GetQi() const = 0;
    virtual int GetMaxQi() const = 0;

    virtual void TakeDamage(int attackValue) = 0;
    virtual void Heal(int amount) = 0;
    virtual bool ConsumeQi(int cost) = 0;
    virtual void RestoreQi(int amount) = 0;

    virtual void Attack(ICombatant* target) = 0;

    virtual void ApplyStatus(const StatusEffect& effect) = 0;
};
