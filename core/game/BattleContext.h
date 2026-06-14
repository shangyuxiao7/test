#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../../core/battle/BattleSession.h"

// 一次完整战斗（可能含多波数）的临时上下文
struct BattleContext {
    std::string enemyGroupId;              // 敌群组配置 ID
    int totalWaves = 1;                    // 总波数
    int currentWave = 0;                   // 当前已进行到的波数（0-based）
    std::vector<std::string> playerCharacterIds;  // 按 BattleSession 玩家索引顺序存储 characterId
    std::unique_ptr<BattleSession> session;       // 当前波次的战斗会话
    bool pendingWorldEvent = false;        // 标记是否需要向世界模块 NotifyEventComplete
    std::string triggerEntityId;           // 触发战斗的实体ID（用于战后标记 PROCESSED）
};
