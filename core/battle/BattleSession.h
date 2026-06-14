// ==================== BattleSession.h ====================
#pragma once
#include "GameController.h"
#include <string>
#include <vector>

struct BattleResult {
    bool isOver = false;
    bool playerWon = false;
};

class BattleSession {
public:
    BattleSession();
    ~BattleSession();
    BattleSession(const BattleSession&) = delete;
    BattleSession& operator=(const BattleSession&) = delete;

    void SetDisplayCallback(DisplayCallback callback);
    int AddPlayer(const std::string& name, int hp, int qi, int speed, int attack, int defense);
    void AddPlayerSkill(int playerIndex, const std::string& skillName);
    void AddEnemyGroup(const std::string& groupName);

    void Start();
    void Update();
    bool IsOver() const;
    BattleResult GetResult() const;

    void SubmitMainAction(MainActionType action);
    void SubmitSkill(int skillIndex);
    void SubmitTarget(int targetIndex);

    GameController::Phase GetPhase() const;
    Character* GetCurrentActor() const;
    std::vector<Character*> GetPlayerTeam() const;
    std::vector<Character*> GetEnemyTeam() const;
    std::vector<std::pair<std::string, float>> GetActionOrder() const;
    std::vector<Character*> GetTargetCandidates() const;
    TargetType GetExpectedTargetType() const;
    std::vector<std::string> FlushLogs();

private:
    GameController* controller;
    std::vector<Character*> players;
    DisplayCallback displayCallback;
};
