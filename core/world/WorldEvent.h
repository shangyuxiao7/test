#pragma once
#include <string>

enum class WorldEventType {
    None,
    RequestDialogue,
    RequestBattle,
    RequestLoot,
    RequestTransferMap,
    Notification
};

struct WorldEvent {
    WorldEventType type = WorldEventType::None;
    std::string param1;
    int param2 = 0;
    std::string param3;
};
