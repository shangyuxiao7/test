// ==================== main.cpp ====================
#include "GameController.h"
#include "ConsoleUI.h"
#include "Action.h"
#include "SkillLibrary.h"
#include "EnemyFactory.h"
#include "PlayerCharacter.h"
#include <ctime>
#include <thread>
#include <chrono>

Character* CreatePlayer(const std::string& name, int hp, int qi, int speed, int attack, int defense) {
    Character* hero = new PlayerCharacter(name, Attributes(hp, qi, speed, attack, defense), Faction::Player);
    hero->AddAction(new NormalAttack());
    hero->SetDisplayCallback(ConsoleUI::HandleEvent);
    return hero;
}

void Battle1() {
    GameController game;

    Character* p1 = CreatePlayer("李逍遥", 120, 60, 12, 25, 8);
    p1->AddSkill(SkillLibrary::Fireball());
    p1->AddSkill(SkillLibrary::FlameSlash());

    Character* p2 = CreatePlayer("赵灵儿", 100, 80, 10, 18, 5);
    p2->AddSkill(SkillLibrary::Heal());
    p2->AddSkill(SkillLibrary::TrueYuanProtect());

    game.AddCharacter(p1);
    game.AddCharacter(p2);

    auto enemies = EnemyFactory::Group_Novice();
    for (auto* e : enemies) {
        e->SetDisplayCallback(ConsoleUI::HandleEvent);   // ← 新增：敌人也统一用 ConsoleUI
        game.AddCharacter(e);
    }

    game.StartBattle();

    while (!game.IsBattleOver()) {
        game.Tick();
        ConsoleUI::Render(game);
    }

    game.Cleanup();
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    Battle1();
    return 0;
}