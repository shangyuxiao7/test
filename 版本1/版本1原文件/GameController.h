// ==================== GameController.h ====================
#pragma once
#include "BattleUI.h"
#include "BattleManager.h"
#include "TurnManager.h"
#include "Character.h"
#include "Action.h"
#include <iostream>

// 游戏流程控制器 - 负责驱动回合循环、处理玩家输入、判定胜负
class GameController {
private:
    BattleManager battleManager;
    TurnManager turnManager;
    bool battleEnded = false;

    // 检查战斗是否结束并设置结果
    bool CheckAndHandleVictory() {
        bool allEnemyDead = true;
        for (auto* e : battleManager.GetEnemyTeam()) {
            if (e->IsAlive()) { allEnemyDead = false; break; }
        }
        if (allEnemyDead) {
            BattleUI::ShowBattleResult(true);
            return true;
        }

        bool allPlayerDead = true;
        for (auto* p : battleManager.GetPlayerTeam()) {
            if (p->IsAlive()) { allPlayerDead = false; break; }
        }
        if (allPlayerDead) {
            BattleUI::ShowBattleResult(false);
            return true;
        }
        return false;
    }

    // 处理玩家回合
    void ProcessPlayerTurn(Character* actor) {
        BattleUI::ShowTurnStart(actor->GetName());
        // 显示战场状态（包含行动顺序）
        BattleUI::ShowBattleStatus(
            battleManager.GetPlayerTeam(),
            battleManager.GetEnemyTeam(),
            turnManager.GetQueue(),
            turnManager.GetCurrentIndex()   // 需在 TurnManager 中添加 GetCurrentIndex()
        );
        std::cout << "当前状态：HP " << actor->GetHp() << "/" << actor->GetMaxHp()
            << "  Qi " << actor->GetQi() << "/" << actor->GetMaxQi() << "\n";

        auto actions = actor->GetActions();
        int actionIndex = BattleUI::SelectAction(actions);
        Action* selectedAction = actions[actionIndex];

        auto& enemies = battleManager.GetEnemyTeam();
        int targetIndex = BattleUI::SelectTarget(enemies);
        if (targetIndex < 0) return;

        Character* target = enemies[targetIndex];
        selectedAction->Execute(actor, target);
    }

    // 处理敌方回合
    void ProcessEnemyTurn(Character* actor) {
        actor->TakeTurn();
        // 敌方回合也可以显示行动顺序（可选）
        BattleUI::ShowBattleStatus(
            battleManager.GetPlayerTeam(),
            battleManager.GetEnemyTeam(),
            turnManager.GetQueue(),
            turnManager.GetCurrentIndex()
        );

        auto& targets = battleManager.GetPlayerTeam();
        Character* target = actor->SelectTarget(targets);

        if (target && target->IsAlive()) {
            std::cout << "\n>> " << actor->GetName() << " 自动攻击 "
                << target->GetName() << "！\n";
            actor->PerformAttack(target);
        }
    }

public:
    void AddCharacter(Character* character) {
        battleManager.AddCharacter(character);
    }

    void StartBattle() {
        std::vector<Character*> allChars;
        for (auto* p : battleManager.GetPlayerTeam()) allChars.push_back(p);
        for (auto* e : battleManager.GetEnemyTeam()) allChars.push_back(e);
        turnManager.Initialize(allChars);

        BattleUI::ShowBattleStart(battleManager.GetPlayerTeam(), battleManager.GetEnemyTeam());
        BattleUI::Pause();

        while (!battleEnded) {
            Character* actor = turnManager.GetNextActor();
            if (!actor) {
                turnManager.ResetRound();
                actor = turnManager.GetNextActor();
                if (!actor) break;
            }

            if (!actor->IsAlive()) continue;

            if (actor->GetFaction() == Faction::Player) {
                ProcessPlayerTurn(actor);
            }
            else {
                ProcessEnemyTurn(actor);
            }

            BattleUI::Pause();

            if (CheckAndHandleVictory()) {
                battleEnded = true;
            }
        }
    }

    void Cleanup() {
        battleManager.Cleanup();
    }
};