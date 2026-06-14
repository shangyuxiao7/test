// ==================== BattleSession.cpp ====================
#include "BattleSession.h"
#include "PlayerCharacter.h"
#include "AICharacter.h"
#include "EnemyFactory.h"
#include "SkillLibrary.h"
#include "Action.h"

BattleSession::BattleSession() : controller(new GameController()), displayCallback(nullptr) {}

BattleSession::~BattleSession() {
    if (controller) {
        controller->Cleanup();
        delete controller;
        controller = nullptr;
    }
}

void BattleSession::SetDisplayCallback(DisplayCallback callback) {
    displayCallback = callback;
}

int BattleSession::AddPlayer(const std::string& name, int hp, int qi, int speed, int attack, int defense) {
    Character* hero = new PlayerCharacter(name, Attributes(hp, qi, speed, attack, defense), Faction::Player);
    hero->AddAction(new NormalAttack());
    if (displayCallback) hero->SetDisplayCallback(displayCallback);
    controller->AddCharacter(hero);
    players.push_back(hero);
    return static_cast<int>(players.size()) - 1;
}

void BattleSession::AddPlayerSkill(int playerIndex, const std::string& skillName) {
    if (playerIndex < 0 || playerIndex >= static_cast<int>(players.size())) return;
    Action* skill = SkillLibrary::Create(skillName);
    if (skill) players[playerIndex]->AddSkill(skill);
}

void BattleSession::AddEnemyGroup(const std::string& groupName) {
    std::vector<Character*> enemies = EnemyFactory::CreateGroup(groupName);
    for (Character* e : enemies) {
        if (displayCallback) e->SetDisplayCallback(displayCallback);
        controller->AddCharacter(e);
    }
}

void BattleSession::Start() { controller->StartBattle(); }
void BattleSession::Update() { controller->Tick(); }
bool BattleSession::IsOver() const { return controller->IsBattleOver(); }

BattleResult BattleSession::GetResult() const {
    BattleResult result;
    result.isOver = controller->IsBattleOver();
    if (result.isOver) result.playerWon = (controller->GetWinner() == Faction::Player);
    return result;
}

void BattleSession::SubmitMainAction(MainActionType action) { controller->SubmitMainAction(action); }
void BattleSession::SubmitSkill(int skillIndex) { controller->SubmitSkill(skillIndex); }
void BattleSession::SubmitTarget(int targetIndex) { controller->SubmitTarget(targetIndex); }

GameController::Phase BattleSession::GetPhase() const { return controller->GetPhase(); }
Character* BattleSession::GetCurrentActor() const { return controller->GetCurrentActor(); }
std::vector<Character*> BattleSession::GetPlayerTeam() const { return controller->GetPlayerTeam(); }
std::vector<Character*> BattleSession::GetEnemyTeam() const { return controller->GetEnemyTeam(); }
std::vector<std::pair<std::string, float>> BattleSession::GetActionOrder() const { return controller->GetActionOrder(); }
std::vector<Character*> BattleSession::GetTargetCandidates() const { return controller->GetTargetCandidates(); }
TargetType BattleSession::GetExpectedTargetType() const { return controller->GetExpectedTargetType(); }
std::vector<std::string> BattleSession::FlushLogs() { return controller->FlushLogs(); }
