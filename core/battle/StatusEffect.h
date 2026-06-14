// ==================== StatusEffect.h ====================
#pragma once
#include <string>

enum class EffectType {
    AttackBuff,
    DefenseBuff,
    AttackDebuff,
    DefenseDebuff,
    HoT,
    DoT
};

struct StatusEffect {
    std::string name;
    EffectType type;
    int value;
    int duration;
    int maxDuration;

    StatusEffect() : name(""), type(EffectType::AttackBuff), value(0), duration(0), maxDuration(0) {}
    StatusEffect(const std::string& name, EffectType type, int value, int duration)
        : name(name), type(type), value(value), duration(duration), maxDuration(duration) {
    }

    bool IsExpired() const { return duration <= 0; }

    std::string GetTypeString() const {
        switch (type) {
        case EffectType::AttackBuff: return "攻↑";
        case EffectType::DefenseBuff: return "防↑";
        case EffectType::AttackDebuff: return "攻↓";
        case EffectType::DefenseDebuff: return "防↓";
        case EffectType::HoT: return "疗";
        case EffectType::DoT: return "毒";
        default: return "?";
        }
    }
};
