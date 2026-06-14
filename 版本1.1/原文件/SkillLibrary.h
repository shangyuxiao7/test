#pragma once
// ==================== SkillLibrary.h ====================
#pragma once
#include "Action.h"

// 技能库 - 预制各种技能，返回 Action*（由 Character 接管内存）
class SkillLibrary {
public:
    // ========== 伤害类 ==========
    static Action* Fireball() {
        return new DamageSkill("火球术", 15, 35);
    }
    static Action* IceArrow() {
        return new DamageSkill("冰箭术", 12, 28);
    }
    static Action* ThunderStrike() {
        return new DamageSkill("雷击", 20, 45);
    }
    static Action* ShadowBolt() {
        return new DamageSkill("暗影箭", 12, 30);
    }
    static Action* Slash() {
        return new DamageSkill("斩击", 8, 20);
    }
    static Action* HeavyStrike() {
        return new DamageSkill("重击", 18, 40);
    }
    static Action* PoisonDart() {
        // 伤害 + 中毒
        return new CompositeSkill("毒镖", 15, 20, StatusEffect("中毒", EffectType::DoT, 4, 3), TargetType::Enemy);
    }

    // ========== 治疗类 ==========
    static Action* Heal() {
        return new HealSkill("治疗术", 10, 30);
    }
    static Action* GreatHeal() {
        return new HealSkill("大治疗术", 20, 60);
    }
    static Action* Regeneration() {
        // 持续恢复（HoT buff）
        return new BuffSkill("再生", 12, StatusEffect("持续恢复", EffectType::HoT, 8, 3), TargetType::Ally);
    }

    // ========== Buff 类（增益） ==========
    static Action* BattleCry() {
        // 自身攻击提升
        return new BuffSkill("战吼", 10, StatusEffect("攻击提升", EffectType::AttackBuff, 10, 3), TargetType::Self);
    }
    static Action* TrueYuanProtect() {
        // 友方防御提升（真元护体）
        return new BuffSkill("真元护体", 15, StatusEffect("真元护体", EffectType::DefenseBuff, 8, 3), TargetType::Ally);
    }
    static Action* Fortify() {
        // 自身防御提升
        return new BuffSkill("硬化", 8, StatusEffect("防御提升", EffectType::DefenseBuff, 8, 2), TargetType::Self);
    }
    static Action* BloodRage() {
        // 自身攻击大幅提升
        return new BuffSkill("狂血", 10, StatusEffect("狂血", EffectType::AttackBuff, 15, 2), TargetType::Self);
    }

    // ========== Debuff 类（减益） ==========
    static Action* Weaken() {
        // 敌方攻击降低
        return new BuffSkill("虚弱", 12, StatusEffect("虚弱", EffectType::AttackDebuff, 10, 3), TargetType::Enemy);
    }
    static Action* Curse() {
        // 敌方防御降低
        return new BuffSkill("诅咒", 12, StatusEffect("诅咒", EffectType::DefenseDebuff, 8, 3), TargetType::Enemy);
    }

    // ========== 组合类（伤害 + 附加效果） ==========
    static Action* FlameSlash() {
        // 伤害 + 灼烧
        return new CompositeSkill("烈焰斩", 20, 25, StatusEffect("灼烧", EffectType::DoT, 5, 3), TargetType::Enemy);
    }
    static Action* VenomStrike() {
        // 伤害 + 剧毒
        return new CompositeSkill("毒蛇打击", 18, 22, StatusEffect("剧毒", EffectType::DoT, 6, 4), TargetType::Enemy);
    }
    static Action* HolySmite() {
        // 伤害 + 致盲（攻击降低）
        return new CompositeSkill("圣光打击", 25, 30, StatusEffect("致盲", EffectType::AttackDebuff, 10, 2), TargetType::Enemy);
    }
};