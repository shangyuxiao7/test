#include "GameManager.h"
#include "BattleRewardTable.h"
#include <cstdlib>
#include <algorithm>

void GameManager::Initialize(PartyManager* party, InventoryManager* inventory, WorldFacade* world) {
    party_ = party;
    inventory_ = inventory;
    world_ = world;
}

void GameManager::SetBattleDisplayCallback(BattleDisplayCallback callback) {
    battleCallback_ = callback;
}

void GameManager::SetGameEventCallback(GameEventCallback callback) {
    gameEventCallback_ = callback;
}

bool GameManager::HandleWorldEvent(const WorldEvent& event) {
    if (event.type == WorldEventType::RequestBattle) {
        int waves = event.param2;
        if (waves <= 0) waves = 1;
        StartBattle(event.param1, waves, event.param3);
        return true;
    }
    if (event.type == WorldEventType::RequestLoot) {
        HandleLootEvent(event);
        return true;
    }
    if (event.type == WorldEventType::RequestDialogue) {
        HandleDialogueEvent(event);
        return true;
    }
    if (event.type == WorldEventType::RequestTransferMap) {
        HandleTransferMapEvent(event);
        return true;
    }
    return false;
}

void GameManager::HandleLootEvent(const WorldEvent& event) {
    if (gameEventCallback_ && !event.param1.empty()) {
        std::string msg = "[获得] " + event.param1;
        if (event.param2 > 1) msg += " x" + std::to_string(event.param2);
        gameEventCallback_(msg);
    }
    if (inventory_ && !event.param1.empty()) {
        int count = event.param2 > 0 ? event.param2 : 1;
        inventory_->AddItem(event.param1, count);
    }
    if (world_) {
        world_->NotifyEventComplete(true);
        if (!event.param3.empty()) {
            world_->MarkEntityProcessed(event.param3);
        }
    }
}

void GameManager::HandleDialogueEvent(const WorldEvent& event) {
    if (gameEventCallback_ && !event.param1.empty()) {
        gameEventCallback_("[剧情] " + event.param1);
    }
    if (party_ && !event.param1.empty() && event.param1.find("join_") == 0) {
        std::string characterId = event.param1.substr(5);
        if (!characterId.empty()) {
            if (gameEventCallback_) {
                gameEventCallback_("[入队] " + characterId + " 加入了队伍！");
            }
            party_->AddMember(characterId, 1);
        }
    }
    if (world_) {
        world_->NotifyEventComplete(true);
        if (!event.param3.empty()) {
            world_->MarkEntityProcessed(event.param3);
        }
    }
}

void GameManager::HandleTransferMapEvent(const WorldEvent& event) {
    if (world_ && !event.param1.empty() && !event.param3.empty()) {
        world_->EnterMap(event.param1, event.param3);
    }
    if (world_) {
        world_->NotifyEventComplete(true);
        if (!event.param3.empty()) {
            world_->MarkEntityProcessed(event.param3);
        }
    }
}

void GameManager::StartBattle(const std::string& enemyGroupId, int waves, const std::string& entityId) {
    battle_ = std::make_unique<BattleContext>();
    battle_->enemyGroupId = enemyGroupId;
    battle_->totalWaves = waves;
    battle_->currentWave = 0;
    battle_->pendingWorldEvent = true;
    battle_->triggerEntityId = entityId;

    // 导出活跃成员并建立 ID 映射
    auto initData = party_->ExportActiveMembersForBattle();
    battle_->playerCharacterIds.clear();
    for (const auto& data : initData) {
        battle_->playerCharacterIds.push_back(data.characterId);
    }

    SetupCurrentWave();
}

bool GameManager::SetupCurrentWave() {
    if (!battle_) return false;
    battle_->session = std::make_unique<BattleSession>();
    if (battleCallback_) {
        battle_->session->SetDisplayCallback(battleCallback_);
    }

    // 重新注入玩家。注意：如果是后续波次，应使用 PartyManager 中当前状态（已同步上一波伤害）
    auto initData = party_->ExportActiveMembersForBattle();
    for (const auto& data : initData) {
        int idx = battle_->session->AddPlayer(
            data.name, data.hp, data.qi, data.speed, data.attack, data.defense
        );
        for (const auto& skill : data.skillIds) {
            battle_->session->AddPlayerSkill(idx, skill);
        }
    }

    battle_->session->AddEnemyGroup(battle_->enemyGroupId);
    battle_->session->Start();
    return true;
}

bool GameManager::IsInBattle() const {
    return battle_ != nullptr;
}

GameController::Phase GameManager::GetBattlePhase() const {
    if (!battle_ || !battle_->session) return GameController::Phase::Idle;
    return battle_->session->GetPhase();
}

Character* GameManager::GetCurrentBattleActor() const {
    if (!battle_ || !battle_->session) return nullptr;
    return battle_->session->GetCurrentActor();
}

BattleSession* GameManager::GetCurrentBattleSession() const {
    if (!battle_) return nullptr;
    return battle_->session.get();
}

std::vector<Character*> GameManager::GetBattleTargetCandidates() const {
    if (!battle_ || !battle_->session) return {};
    return battle_->session->GetTargetCandidates();
}

TargetType GameManager::GetBattleExpectedTargetType() const {
    if (!battle_ || !battle_->session) return TargetType::None;
    return battle_->session->GetExpectedTargetType();
}

void GameManager::SubmitBattleMainAction(MainActionType action) {
    if (battle_ && battle_->session) battle_->session->SubmitMainAction(action);
}

void GameManager::SubmitBattleSkill(int skillIndex) {
    if (battle_ && battle_->session) battle_->session->SubmitSkill(skillIndex);
}

void GameManager::SubmitBattleTarget(int targetIndex) {
    if (battle_ && battle_->session) battle_->session->SubmitTarget(targetIndex);
}

bool GameManager::BattleTick() {
    if (!battle_ || !battle_->session) return true;

    auto* session = battle_->session.get();
    session->Update();

    if (!session->IsOver()) return false;

    // 当前波次结束
    auto result = session->GetResult();
    if (!result.playerWon) {
        // 玩家失败，整个战斗结束
        return true;
    }

    // 胜利：检查是否还有下一波
    battle_->currentWave++;
    if (battle_->currentWave >= battle_->totalWaves) {
        return true;  // 全部波次完成
    }

    // 多波数：同步当前状态到队伍（不含奖励），然后开启下一波
    // 这里先同步 HP/Qi/Dead，经验奖励等最后统一发
    SyncPartyAfterBattle(0);
    SetupCurrentWave();
    return false;
}

BattleResult GameManager::GetBattleResult() const {
    if (!battle_ || !battle_->session) return BattleResult{true, false};
    return battle_->session->GetResult();
}

void GameManager::FinalizeBattle(bool eventSuccess) {
    if (!battle_) return;

    auto result = battle_->session->GetResult();

    // 1. 计算并发放奖励（仅胜利时）
    int expPerAliveMember = 0;
    if (result.playerWon) {
        expPerAliveMember = GrantBattleRewards();
    }

    // 2. 同步角色状态（含经验）回 PartyManager
    SyncPartyAfterBattle(expPerAliveMember);

    // 3. 通知世界模块事件已处理
    if (battle_->pendingWorldEvent && world_) {
        world_->NotifyEventComplete(eventSuccess);
    }

    // 4. 标记触发实体为已处理（如果是通过实体交互触发的战斗）
    if (eventSuccess && world_ && !battle_->triggerEntityId.empty()) {
        world_->MarkEntityProcessed(battle_->triggerEntityId);
    }

    // 5. 清理战斗上下文
    battle_.reset();
}

int GameManager::GrantBattleRewards() {
    if (!inventory_ || !battle_) return 0;

    const auto* reward = BattleRewardTable::Instance().GetReward(battle_->enemyGroupId);
    if (!reward) return 0;

    // 发放金钱
    if (reward->gold > 0) {
        inventory_->AddGold(reward->gold);
    }

    // 发放掉落（概率判定）
    for (const auto& drop : reward->drops) {
        float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (roll <= drop.probability) {
            inventory_->AddItem(drop.itemId, drop.count);
        }
    }

    // 计算经验：平分给所有存活成员
    auto team = battle_->session->GetPlayerTeam();
    int aliveCount = 0;
    for (auto* c : team) {
        if (c->GetHp() > 0) ++aliveCount;
    }
    if (aliveCount > 0 && reward->exp > 0) {
        return reward->exp / aliveCount;
    }
    return 0;
}

void GameManager::SyncPartyAfterBattle(int expPerAliveMember) {
    if (!party_ || !battle_ || !battle_->session) return;

    auto team = battle_->session->GetPlayerTeam();
    std::vector<BattleMemberState> states;

    for (size_t i = 0; i < team.size() && i < battle_->playerCharacterIds.size(); ++i) {
        BattleMemberState state;
        state.characterId = battle_->playerCharacterIds[i];
        state.remainingHp = team[i]->GetHp();
        state.remainingQi = team[i]->GetQi();
        state.isDead = (team[i]->GetHp() <= 0);
        state.expGained = (team[i]->GetHp() > 0) ? expPerAliveMember : 0;
        states.push_back(state);
    }

    party_->SyncAfterBattle(states);
}
