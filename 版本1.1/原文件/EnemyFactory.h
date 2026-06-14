// ==================== EnemyFactory.h ====================
#pragma once
#include "Character.h"
#include "AICharacter.h"
#include "Action.h"
#include "SkillLibrary.h"
#include <vector>

class EnemyFactory {
private:
    static Character* CreateBase(const std::string& name, const Attributes& attr) {
        Character* e = new AICharacter(name, attr, Faction::Enemy);
        e->AddAction(new NormalAttack());
        // 不再在这里设置 DisplayCallback，由调用方（main.cpp）统一设置
        return e;
    }

public:
    // ========== 单体敌人预制体 ==========

    static Character* MiaoRenLouLuo() {
        Character* e = CreateBase("苗人喽啰", Attributes(80, 30, 10, 18, 3));
        e->AddSkill(SkillLibrary::Slash());
        return e;
    }

    static Character* MiaoRenTouLing() {
        Character* e = CreateBase("苗人头领", Attributes(150, 40, 8, 22, 10));
        e->AddSkill(SkillLibrary::HeavyStrike());
        e->AddSkill(SkillLibrary::Fortify());
        return e;
    }

    static Character* DuZhiZhu() {
        Character* e = CreateBase("毒蜘蛛", Attributes(70, 50, 14, 30, 2));
        e->AddSkill(SkillLibrary::PoisonDart());
        e->AddSkill(SkillLibrary::VenomStrike());
        return e;
    }

    static Character* GhostSoldier() {
        Character* e = CreateBase("鬼兵", Attributes(100, 40, 12, 20, 6));
        e->AddSkill(SkillLibrary::ShadowBolt());
        e->AddSkill(SkillLibrary::Curse());
        return e;
    }

    static Character* Bandit() {
        Character* e = CreateBase("山贼", Attributes(90, 35, 11, 19, 4));
        e->AddSkill(SkillLibrary::Slash());
        e->AddSkill(SkillLibrary::Weaken());
        return e;
    }

    static Character* DarkMage() {
        Character* e = CreateBase("黑魔法师", Attributes(60, 80, 9, 15, 3));
        e->AddSkill(SkillLibrary::Fireball());
        e->AddSkill(SkillLibrary::ShadowBolt());
        e->AddSkill(SkillLibrary::Curse());
        return e;
    }

    static Character* PoisonBeast() {
        Character* e = CreateBase("毒兽", Attributes(120, 45, 13, 25, 5));
        e->AddSkill(SkillLibrary::PoisonDart());
        e->AddSkill(SkillLibrary::Regeneration());
        return e;
    }

    // ========== 敌人波次/群组（堆） ==========

    // 新手村第一波：2个喽啰
    static std::vector<Character*> Group_Novice() {
        return std::vector<Character*>{ MiaoRenLouLuo(), MiaoRenLouLuo() };
    }

    // 精英战：头领 + 喽啰
    static std::vector<Character*> Group_Elite() {
        return std::vector<Character*>{ MiaoRenTouLing(), MiaoRenLouLuo() };
    }

    // 混合战：毒蜘蛛 + 鬼兵
    static std::vector<Character*> Group_Mixed() {
        return std::vector<Character*>{ DuZhiZhu(), GhostSoldier() };
    }

    // 法师队：黑魔法师 + 山贼
    static std::vector<Character*> Group_MageTeam() {
        return std::vector<Character*>{ DarkMage(), Bandit() };
    }

    // Boss战：头领 + 2喽啰
    static std::vector<Character*> Group_Boss() {
        return std::vector<Character*>{ MiaoRenTouLing(), MiaoRenLouLuo(), MiaoRenLouLuo() };
    }

    // 剧毒之巢：毒蜘蛛 + 毒兽
    static std::vector<Character*> Group_PoisonNest() {
        return std::vector<Character*>{ DuZhiZhu(), PoisonBeast() };
    }

    // 全面进攻：头领 + 黑魔法师 + 山贼
    static std::vector<Character*> Group_FullAssault() {
        return std::vector<Character*>{ MiaoRenTouLing(), DarkMage(), Bandit() };
    }
};