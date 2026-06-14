#pragma once
#include <string>

enum class EntityType { NPC, Chest, Monster, Portal, Trigger, Interactive };

struct EntityDef {
    std::string id;
    std::string name;
    std::string description;
    EntityType type = EntityType::NPC;
    std::string eventId;
    bool isGlobalState = false;
    // 仅 Portal 类型使用：跨地图传送目标
    std::string targetMapId;
    std::string targetRoomId;
};
