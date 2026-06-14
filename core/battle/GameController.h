// ==================== GameController.h ====================
#pragma once
#include "BattleManager.h"
#include "Character.h"
#include "Action.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <cstdlib>

enum class MainActionType { Attack = 1, Skill, RestoreQi };

class GameController {
public:
    enum class Phase { Idle, PlayerMainMenu, PlayerSkillMenu, PlayerTargetSelect, Executing, EnemyTurn, BattleOver };

private:
    BattleManager battleManager;
    bool battleEnded = false;

    struct ActionGaugeSystem {
        static constexpr float GAUGE_MAX = 100000.0f;
        static constexpr float TIME_SCALE = 100.0f;
        std::unordered_map<Character*, float> gauges;

        void Initialize(Character* c) { gauges[c] = 0.0f; }

        float AdvanceTime() {
            float minTime = FLT_MAX;
            for (auto& kv : gauges) {
                Character* c = kv.first;
                float& gauge = kv.second;
                if (!c->IsAlive()) continue;
                float speed = static_cast<float>(c->GetSpeed());
                if (speed <= 0) continue;
                float time = (GAUGE_MAX - gauge) / speed;
                if (time < minTime) minTime = time;
            }
            if (minTime == FLT_MAX) return 0.0f;
            for (auto& kv : gauges) {
                Character* c = kv.first;
                float& gauge = kv.second;
                if (!c->IsAlive()) continue;
                float speed = static_cast<float>(c->GetSpeed());
                gauge += speed * minTime;
            }
            return minTime;
        }

        std::vector<Character*> GetReadyActors() {
            std::vector<Character*> ready;
            for (auto& kv : gauges) {
                Character* c = kv.first;
                float gauge = kv.second;
                if (c->IsAlive() && gauge >= GAUGE_MAX - 1e-3f) ready.push_back(c);
            }
            std::sort(ready.begin(), ready.end(), [](Character* a, Character* b) {
                if (a->GetSpeed() != b->GetSpeed()) return a->GetSpeed() > b->GetSpeed();
                if (a->GetFaction() != b->GetFaction()) return a->GetFaction() == Faction::Player;
                return a < b;
                });
            return ready;
        }

        void ResetActorGauge(Character* actor) {
            auto it = gauges.find(actor);
            if (it != gauges.end()) it->second -= GAUGE_MAX;
        }

        std::vector<std::pair<std::string, float>> GetDisplayOrder() const {
            std::vector<std::pair<Character*, float>> temp;
            for (const auto& kv : gauges) {
                Character* c = kv.first;
                float gauge = kv.second;
                if (!c->IsAlive()) continue;
                float speed = static_cast<float>(c->GetSpeed());
                if (speed <= 0) continue;
                float time = (GAUGE_MAX - gauge) / speed / TIME_SCALE;
                temp.push_back({ c, time });
            }
            std::sort(temp.begin(), temp.end(), [](const auto& a, const auto& b) {
                return a.second < b.second;
                });
            std::vector<std::pair<std::string, float>> result;
            for (const auto& kv : temp) result.push_back({ kv.first->GetName(), kv.second });
            return result;
        }
    };

    ActionGaugeSystem gaugeSystem;

    Phase currentPhase = Phase::Idle;

    Character* currentActor = nullptr;
    MainActionType chosenMainAction = MainActionType::Attack;
    Action* chosenSkillPtr = nullptr;
    std::vector<Character*> targetCandidates;
    TargetType expectedTargetType = TargetType::Enemy;

    std::vector<std::string> logBuffer;
    void PushLog(const std::string& msg) { logBuffer.push_back(msg); }

    bool CheckAndHandleVictory() {
        bool allEnemyDead = true;
        for (auto* e : battleManager.GetEnemyTeam()) {
            if (e->IsAlive()) { allEnemyDead = false; break; }
        }
        if (allEnemyDead) {
            PushLog("========================================");
            PushLog("战斗胜利!");
            PushLog("========================================");
            battleEnded = true;
            return true;
        }
        bool allPlayerDead = true;
        for (auto* p : battleManager.GetPlayerTeam()) {
            if (p->IsAlive()) { allPlayerDead = false; break; }
        }
        if (allPlayerDead) {
            PushLog("========================================");
            PushLog("战斗失败...");
            PushLog("========================================");
            battleEnded = true;
            return true;
        }
        return false;
    }

    void Tick_Idle() {
        if (CheckAndHandleVictory()) { currentPhase = Phase::BattleOver; return; }

        gaugeSystem.AdvanceTime();
        auto ready = gaugeSystem.GetReadyActors();
        if (ready.empty()) return;

        Character* actor = ready[0];
        if (!actor->IsAlive()) {
            gaugeSystem.ResetActorGauge(actor);
            return;
        }

        actor->UpdateEffects();
        if (!actor->IsAlive()) {
            gaugeSystem.ResetActorGauge(actor);
            return;
        }

        if (actor->GetFaction() == Faction::Player) {
            currentActor = actor;
            currentPhase = Phase::PlayerMainMenu;
            PushLog("---------- " + actor->GetName() + " 的回合 ----------");
        }
        else {
            currentActor = actor;
            currentPhase = Phase::EnemyTurn;
        }
    }

    void Tick_EnemyTurn() {
        if (!currentActor) { currentPhase = Phase::Idle; return; }

        auto& targets = battleManager.GetPlayerTeam();
        int decision = rand() % 3;

        if (decision == 0 || currentActor->GetSkills().empty()) {
            Character* target = currentActor->SelectTarget(targets);
            if (target && target->IsAlive()) {
                PushLog(">> " + currentActor->GetName() + " 对 " + target->GetName() + " 发动攻击");
                currentActor->PerformAttack(target);
            }
        }
        else if (decision == 1) {
            auto skills = currentActor->GetSkills();
            std::vector<Action*> availableSkills;
            for (auto* s : skills) if (s->IsAvailable(currentActor)) availableSkills.push_back(s);

            if (!availableSkills.empty()) {
                Action* skill = availableSkills[rand() % availableSkills.size()];
                Character* target = nullptr;
                if (skill->NeedTarget()) {
                    TargetType tType = skill->GetTargetType();
                    if (tType == TargetType::Enemy) target = currentActor->SelectTarget(targets);
                    else if (tType == TargetType::Ally) target = currentActor->SelectTarget(battleManager.GetEnemyTeam());
                    else if (tType == TargetType::Self) target = currentActor;
                }
                if (!skill->NeedTarget() || (target && target->IsAlive())) {
                    std::string msg = ">> " + currentActor->GetName() + " 使用了 " + skill->GetName();
                    if (target) msg += " 对 " + target->GetName();
                    PushLog(msg);
                    skill->Execute(currentActor, target);
                }
            }
            else {
                Character* target = currentActor->SelectTarget(targets);
                if (target && target->IsAlive()) currentActor->PerformAttack(target);
            }
        }
        else {
            currentActor->RestoreQi(20);
            PushLog(">> " + currentActor->GetName() + " restores Qi");
        }

        currentPhase = Phase::Executing;
    }

    void Tick_Executing() {
        if (currentActor) gaugeSystem.ResetActorGauge(currentActor);
        currentActor = nullptr;
        chosenSkillPtr = nullptr;
        currentPhase = Phase::Idle;
    }

public:
    void AddCharacter(Character* character) {
        battleManager.AddCharacter(character);
    }

    void StartBattle() {
        std::vector<Character*> allChars;
        for (auto* p : battleManager.GetPlayerTeam()) allChars.push_back(p);
        for (auto* e : battleManager.GetEnemyTeam()) allChars.push_back(e);
        for (auto* c : allChars) gaugeSystem.Initialize(c);

        PushLog("========================================");
        PushLog("战斗开始!");
        PushLog("========================================");
        currentPhase = Phase::Idle;
    }

    void Tick() {
        switch (currentPhase) {
        case Phase::Idle:          Tick_Idle(); break;
        case Phase::EnemyTurn:     Tick_EnemyTurn(); break;
        case Phase::Executing:     Tick_Executing(); break;
        case Phase::PlayerMainMenu:
        case Phase::PlayerSkillMenu:
        case Phase::PlayerTargetSelect:
            break;
        case Phase::BattleOver:
            break;
        }
    }

    void SubmitMainAction(MainActionType action) {
        if (currentPhase != Phase::PlayerMainMenu || !currentActor) return;
        chosenMainAction = action;

        if (action == MainActionType::Attack) {
            targetCandidates.clear();
            for (auto* e : battleManager.GetEnemyTeam()) if (e->IsAlive()) targetCandidates.push_back(e);
            expectedTargetType = TargetType::Enemy;
            currentPhase = Phase::PlayerTargetSelect;
        }
        else if (action == MainActionType::Skill) {
            currentPhase = Phase::PlayerSkillMenu;
        }
        else if (action == MainActionType::RestoreQi) {
            RestoreQiAction restoreAction(20);
            restoreAction.Execute(currentActor, nullptr);
            currentPhase = Phase::Executing;
        }
    }

    void SubmitSkill(int skillIndex) {
        if (currentPhase != Phase::PlayerSkillMenu || !currentActor) return;
        if (skillIndex == -1) {
            currentPhase = Phase::PlayerMainMenu;
            return;
        }

        auto skills = currentActor->GetSkills();
        if (skillIndex < 0 || skillIndex >= (int)skills.size()) return;
        chosenSkillPtr = skills[skillIndex];

        if (!chosenSkillPtr->IsAvailable(currentActor)) {
            PushLog("技能不可用（气力不足或其他条件）");
            return;
        }

        if (!chosenSkillPtr->NeedTarget()) {
            chosenSkillPtr->Execute(currentActor, nullptr);
            currentPhase = Phase::Executing;
            return;
        }

        TargetType tType = chosenSkillPtr->GetTargetType();
        expectedTargetType = tType;
        targetCandidates.clear();

        if (tType == TargetType::Enemy) {
            for (auto* e : battleManager.GetEnemyTeam()) if (e->IsAlive()) targetCandidates.push_back(e);
        }
        else if (tType == TargetType::Ally) {
            for (auto* p : battleManager.GetPlayerTeam()) if (p->IsAlive()) targetCandidates.push_back(p);
        }
        else if (tType == TargetType::Self) {
            chosenSkillPtr->Execute(currentActor, currentActor);
            currentPhase = Phase::Executing;
            return;
        }

        if (!targetCandidates.empty()) {
            currentPhase = Phase::PlayerTargetSelect;
        }
        else {
            PushLog("没有可选目标！");
            currentPhase = Phase::PlayerSkillMenu;
        }
    }

    void SubmitTarget(int targetIndex) {
        if (currentPhase != Phase::PlayerTargetSelect || !currentActor) return;
        if (targetIndex == -2) {
            if (chosenMainAction == MainActionType::Attack) {
                currentPhase = Phase::PlayerMainMenu;
            }
            else {
                currentPhase = Phase::PlayerSkillMenu;
            }
            return;
        }

        if (targetIndex < 0 || targetIndex >= (int)targetCandidates.size()) return;
        Character* target = targetCandidates[targetIndex];

        if (chosenMainAction == MainActionType::Attack) {
            currentActor->PerformAttack(target);
        }
        else if (chosenMainAction == MainActionType::Skill && chosenSkillPtr) {
            chosenSkillPtr->Execute(currentActor, target);
        }
        currentPhase = Phase::Executing;
    }

    Phase GetPhase() const { return currentPhase; }
    Character* GetCurrentActor() const { return currentActor; }
    std::vector<Character*> GetPlayerTeam() const { return battleManager.GetPlayerTeam(); }
    std::vector<Character*> GetEnemyTeam() const { return battleManager.GetEnemyTeam(); }
    std::vector<std::pair<std::string, float>> GetActionOrder() const { return gaugeSystem.GetDisplayOrder(); }
    bool IsBattleOver() const { return battleEnded; }

    std::vector<Character*> GetTargetCandidates() const { return targetCandidates; }
    TargetType GetExpectedTargetType() const { return expectedTargetType; }

    std::vector<std::string> FlushLogs() {
        auto temp = logBuffer;
        logBuffer.clear();
        return temp;
    }

    void Cleanup() {
        battleManager.Cleanup();
    }
    Faction GetWinner() const { return battleManager.GetWinner(); }
};
