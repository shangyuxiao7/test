// ==================== main.cpp ====================
#include "GameController.h"
#include "Action.h"
#include <ctime>

// 创建玩家角色
Character* CreatePlayer(const std::string& name, int hp, int qi, int speed, int attack, int defense) {
    Character* hero = new PlayerCharacter(name, Attributes(hp, qi, speed, attack, defense), Faction::Player);
    hero->AddAction(new NormalAttack());
    hero->SetDisplayCallback(GameDisplay::Show);
    return hero;
}

// 创建敌方角色
Character* CreateEnemy(const std::string& name, int hp, int qi, int speed, int attack, int defense) {
    Character* enemy = new AICharacter(name, Attributes(hp, qi, speed, attack, defense), Faction::Enemy);
    enemy->AddAction(new NormalAttack());
    enemy->SetDisplayCallback(GameDisplay::Show);
    return enemy;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    GameController game;

    // 创建并添加角色
    game.AddCharacter(CreatePlayer("主角", 120, 60, 12, 25, 8));
    game.AddCharacter(CreatePlayer("队友", 100, 50, 15, 20, 5));
    game.AddCharacter(CreateEnemy("哥布林", 80, 30, 10, 18, 3));
    game.AddCharacter(CreateEnemy("兽人", 150, 20, 8, 22, 10));
    game.AddCharacter(CreateEnemy("黑暗法师", 70, 100, 14, 30, 2));

    // 开始战斗
    game.StartBattle();

    // 清理
    game.Cleanup();

    return 0;
}