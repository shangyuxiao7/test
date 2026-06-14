#pragma once
#include <functional>
#include <memory>
#include <string>
#include "../../core/battle/BattleSession.h"
#include "../../core/character/PartyManager.h"
#include "../../core/inventory/InventoryManager.h"
#include "../../core/world/WorldFacade.h"
#include "BattleContext.h"

enum class BattleOutcome { Victory, Defeat, Fled };

class GameManager {
public:
    using BattleDisplayCallback = std::function<void(const BattleGameEvent&)>;
    using GameEventCallback = std::function<void(const std::string&)>;

    GameManager() = default;
    ~GameManager() = default;

    // 绑定三个子模块（必须在配置加载完成后调用）
    void Initialize(PartyManager* party, InventoryManager* inventory, WorldFacade* world);

    // 绑定战斗事件回调（透传给 BattleSession）
    void SetBattleDisplayCallback(BattleDisplayCallback callback);

    // 绑定通用游戏事件回调（Loot/Dialogue/入队等通知，由 UI 层打印）
    void SetGameEventCallback(GameEventCallback callback);

    // 处理世界事件。如果接管则返回 true；否则返回 false
    bool HandleWorldEvent(const WorldEvent& event);

    // 非战斗事件处理（Loot / Dialogue / TransferMap）
    void HandleLootEvent(const WorldEvent& event);
    void HandleDialogueEvent(const WorldEvent& event);
    void HandleTransferMapEvent(const WorldEvent& event);

    // ---------- 战斗状态查询 ----------
    bool IsInBattle() const;
    GameController::Phase GetBattlePhase() const;
    Character* GetCurrentBattleActor() const;
    BattleSession* GetCurrentBattleSession() const;

    // 获取当前可选目标（供 UI 展示）
    std::vector<Character*> GetBattleTargetCandidates() const;
    TargetType GetBattleExpectedTargetType() const;

    // ---------- 战斗输入提交 ----------
    void SubmitBattleMainAction(MainActionType action);
    void SubmitBattleSkill(int skillIndex);
    void SubmitBattleTarget(int targetIndex);

    // ---------- 战斗驱动 ----------
    // 驱动一帧。返回 true 表示整个战斗（含所有波数）已完全结束
    bool BattleTick();

    // 获取最终战斗结果（仅在返回 true 后有效）
    BattleResult GetBattleResult() const;

    // 战后结算：同步角色状态、发放奖励、通知世界模块。
    // eventSuccess: 是否成功处理（胜利传 true，逃跑/失败传 false，由调用方决定剧情走向）
    void FinalizeBattle(bool eventSuccess);

private:
    void StartBattle(const std::string& enemyGroupId, int waves, const std::string& entityId = "");
    bool SetupCurrentWave();   // 根据当前波数配置 BattleSession
    void SyncPartyAfterBattle(int expPerAliveMember);
    int GrantBattleRewards();

    PartyManager* party_ = nullptr;
    InventoryManager* inventory_ = nullptr;
    WorldFacade* world_ = nullptr;
    BattleDisplayCallback battleCallback_;
    GameEventCallback gameEventCallback_;
    std::unique_ptr<BattleContext> battle_;
};
