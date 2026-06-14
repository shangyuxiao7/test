// ==================== GameEvent.h ====================
#pragma once
#include <string>
#include <functional>

enum class Faction { Player, Enemy };

enum class BattleEventType {
    DamageTaken,
    Healed,
    QiConsumed,
    QiRestored,
    Died,
    TurnStart,
    Attack,
    BattleWin,
    BattleLose,
    BattleStart,
    BuffApplied,
    BuffExpired,
    DotTick,
    StatChanged
};

struct BattleGameEvent {
    BattleEventType type;
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

using DisplayCallback = std::function<void(const BattleGameEvent&)>;
