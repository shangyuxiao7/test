// ==================== GameEvent.h ====================
#pragma once
#include <string>
#include <functional>

// 阵营枚举 - 最基础定义
enum class Faction { Player, Enemy };

// 事件类型
enum class EventType {
    DamageTaken,
    Healed,
    QiConsumed,
    QiRestored,
    Died,
    TurnStart,
    Attack,
    BattleWin,
    BattleLose,
    BattleStart
};

// 事件数据结构 - 从计算层传递到显示层
struct GameEvent {
    EventType type;
    std::string characterName;
    std::string targetName;
    int value;
    int currentHp;
    int currentQi;
    int maxHp;
    int attack;
    int defense;
    int speed;
    std::string message;
};

// 显示回调类型
using DisplayCallback = std::function<void(const GameEvent&)>;