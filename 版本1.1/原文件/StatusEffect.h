// ==================== StatusEffect.h ====================
#pragma once
#include <string>

// 状态效果类型
enum class EffectType {
    AttackBuff,    // 攻击提升
    DefenseBuff,   // 防御提升
    AttackDebuff,  // 攻击降低
    DefenseDebuff, // 防御降低
    HoT,           // 持续治疗（Heal over Time）
    DoT            // 持续伤害（Damage over Time）
};

// 状态效果数据结构
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