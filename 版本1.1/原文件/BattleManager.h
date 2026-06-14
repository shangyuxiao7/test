// ==================== BattleManager.h ====================
#pragma once
#include "TurnManager.h"

// 战斗管理器 - 总控战斗流程
class BattleManager {
private:
    std::vector<Character*> playerTeam;
    std::vector<Character*> enemyTeam;
    std::vector<Character*> allCharacters;
    TurnManager turnManager;
    bool battleEnd = false;
    Faction winnerFaction;

public:
    void AddCharacter(Character* character) {
        allCharacters.push_back(character);
        if (character->GetFaction() == Faction::Player) {
            playerTeam.push_back(character);
        }
        else {
            enemyTeam.push_back(character);
        }
    }
    std::vector<Character*>& GetPlayerTeam() { return playerTeam; }
    std::vector<Character*>& GetEnemyTeam() { return enemyTeam; }
    const std::vector<Character*>& GetPlayerTeam() const { return playerTeam; }
    const std::vector<Character*>& GetEnemyTeam() const { return enemyTeam; }

    void StartBattle() {
        turnManager.Initialize(allCharacters);
        RunBattleLoop();
    }

    void RunBattleLoop() {
        while (!battleEnd) {
            Character* actor = turnManager.GetNextActor();
            if (!actor) break;
            ExecuteTurn(actor);
            if (CheckVictory()) {
                battleEnd = true;
            }
        }
    }

    void ExecuteTurn(Character* actor) {
        actor->TakeTurn();
        auto& enemies = (actor->GetFaction() == Faction::Player) ? enemyTeam : playerTeam;
        Character* target = actor->SelectTarget(enemies);
        if (target && target->IsAlive()) {
            actor->PerformAttack(target);
        }
    }

    bool CheckVictory() {
        bool allEnemyDead = true;
        for (auto* e : enemyTeam) {
            if (e->IsAlive()) { allEnemyDead = false; break; }
        }
        if (allEnemyDead) {
            winnerFaction = Faction::Player;
            return true;
        }

        bool allPlayerDead = true;
        for (auto* p : playerTeam) {
            if (p->IsAlive()) { allPlayerDead = false; break; }
        }
        if (allPlayerDead) {
            winnerFaction = Faction::Enemy;
            return true;
        }
        return false;
    }

    Faction GetWinner() const { return winnerFaction; }

    void Cleanup() {
        for (auto* c : allCharacters) delete c;
        allCharacters.clear();
        playerTeam.clear();
        enemyTeam.clear();
    }
};