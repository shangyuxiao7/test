#include "WorldManager.h"
#include "../../foundation/JsonUtils.h"

WorldManager::WorldManager() = default;

void WorldManager::SetMapData(MapData* data) { mapData = data; }
void WorldManager::SetEventCallback(WorldEventCallback cb) { eventCallback = std::move(cb); }

void WorldManager::EnterMap(const std::string& mapId, const std::string& entryRoomId) {
    if (!mapData || !mapData->HasMap(mapId)) return;
    currentMapId = mapId;
    currentRoomId = entryRoomId;
    auto it = mapInstances.find(mapId);
    if (it == mapInstances.end()) {
        mapInstances[mapId] = std::make_unique<MapInstance>(mapId);
    }
    ExecuteRoomPipeline();
}

void WorldManager::LeaveMap() {
    currentMapId.clear();
    currentRoomId.clear();
    currentPipeline.clear();
    pipelineIndex = 0;
    waitingForEvent = false;
}

bool WorldManager::MoveToRoom(int exitIndex) {
    if (waitingForEvent) return false;
    const RoomDef* room = GetCurrentRoomDef();
    if (!room || exitIndex < 0 || exitIndex >= static_cast<int>(room->exits.size())) return false;

    const ExitDef& exit = room->exits[exitIndex];
    if (!exit.conditionFlag.empty()) {
        auto it = globalFlags.find(exit.conditionFlag);
        if (it == globalFlags.end() || it->second == 0) return false;
    }

    if (!exit.targetMapId.empty() && exit.targetMapId != currentMapId) {
        EnterMap(exit.targetMapId, exit.targetRoomId);
    } else {
        currentRoomId = exit.targetRoomId;
        ExecuteRoomPipeline();
    }
    return true;
}

bool WorldManager::InteractWithEntity(int entityIndex) {
    if (waitingForEvent) return false;
    const RoomDef* room = GetCurrentRoomDef();
    if (!room || entityIndex < 0 || entityIndex >= static_cast<int>(room->entities.size())) return false;

    const EntityDef& entity = room->entities[entityIndex];
    if (IsEntityProcessed(currentRoomId, entity.id)) return false;

    switch (entity.type) {
        case EntityType::Chest:
            pendingEvent = WorldEvent{WorldEventType::RequestLoot, entity.eventId, 1, entity.id};
            break;
        case EntityType::Monster:
            pendingEvent = WorldEvent{WorldEventType::RequestBattle, entity.eventId, 1, entity.id};
            break;
        case EntityType::NPC:
            pendingEvent = WorldEvent{WorldEventType::RequestDialogue, entity.eventId, 0, entity.id};
            break;
        case EntityType::Portal:
            pendingEvent = WorldEvent{WorldEventType::RequestTransferMap, entity.targetMapId, 0, entity.targetRoomId};
            break;
        default:
            pendingEvent = WorldEvent{WorldEventType::Notification, "interacted with " + entity.name, 0, entity.id};
            break;
    }

    waitingForEvent = true;
    if (eventCallback) eventCallback(pendingEvent);
    return true;
}

bool WorldManager::HasPendingEvent() const { return waitingForEvent; }
WorldEvent WorldManager::GetPendingEvent() const { return pendingEvent; }

void WorldManager::NotifyEventComplete(bool success) {
    if (!waitingForEvent) return;
    waitingForEvent = false;

    if (pendingEvent.type == WorldEventType::RequestLoot && !pendingEvent.param3.empty()) {
        MarkEntityProcessed(pendingEvent.param3);
    }

    AdvancePipeline();
}

WorldManager::RoomInfo WorldManager::GetCurrentRoomInfo() const {
    RoomInfo info;
    const RoomDef* room = GetCurrentRoomDef();
    if (room) {
        info.id = room->id;
        info.name = room->name;
        info.description = room->description;
    }
    return info;
}

std::vector<WorldManager::ExitInfo> WorldManager::GetAvailableExits() const {
    std::vector<ExitInfo> result;
    const RoomDef* room = GetCurrentRoomDef();
    if (!room) return result;

    for (const auto& exit : room->exits) {
        ExitInfo info;
        info.targetRoomId = exit.targetRoomId;
        info.targetMapId = exit.targetMapId;
        info.description = exit.description;
        info.isAvailable = true;
        if (!exit.conditionFlag.empty()) {
            auto it = globalFlags.find(exit.conditionFlag);
            info.isAvailable = (it != globalFlags.end() && it->second != 0);
        }
        result.push_back(info);
    }
    return result;
}

std::vector<WorldManager::EntityInfo> WorldManager::GetRoomEntities() const {
    std::vector<EntityInfo> result;
    const RoomDef* room = GetCurrentRoomDef();
    if (!room) return result;

    for (const auto& entity : room->entities) {
        if (IsEntityProcessed(currentRoomId, entity.id)) continue;
        EntityInfo info;
        info.id = entity.id;
        info.name = entity.name;
        info.description = entity.description;
        info.type = entity.type;
        info.isInteractable = (entity.type != EntityType::Trigger);
        result.push_back(info);
    }
    return result;
}

void WorldManager::MarkEntityProcessed(const std::string& entityId) {
    MapInstance* inst = GetCurrentMapInstance();
    if (!inst || currentRoomId.empty()) return;
    const MapDef* map = mapData ? mapData->GetMap(currentMapId) : nullptr;
    if (!map) return;
    for (const auto& roomId : map->roomGraph.GetAllRoomIds()) {
        const RoomDef* room = map->roomGraph.GetRoom(roomId);
        if (!room) continue;
        for (const auto& entity : room->entities) {
            if (entity.id == entityId) {
                if (entity.isGlobalState) {
                    globalEntityStates[entityId] = "PROCESSED";
                } else {
                    inst->SetEntityState(roomId, entityId, "PROCESSED");
                }
                return;
            }
        }
    }
}

void WorldManager::SetGlobalFlag(const std::string& flag, int value) {
    globalFlags[flag] = value;
}

int WorldManager::GetGlobalFlag(const std::string& flag) const {
    auto it = globalFlags.find(flag);
    return (it != globalFlags.end()) ? it->second : 0;
}

std::string WorldManager::GetCurrentMapId() const { return currentMapId; }
std::string WorldManager::GetCurrentRoomId() const { return currentRoomId; }

void WorldManager::Serialize(std::string& out) const {
    JsonValue root(JsonType::Object);
    root.SetField("currentMapId", JsonValue(currentMapId));
    root.SetField("currentRoomId", JsonValue(currentRoomId));

    JsonValue flags(JsonType::Object);
    for (const auto& pair : globalFlags) {
        flags.SetField(pair.first, JsonValue(pair.second));
    }
    root.SetField("globalFlags", flags);

    JsonValue globalEntities(JsonType::Object);
    for (const auto& pair : globalEntityStates) {
        globalEntities.SetField(pair.first, JsonValue(pair.second));
    }
    root.SetField("globalEntityStates", globalEntities);

    JsonValue instances(JsonType::Object);
    for (const auto& pair : mapInstances) {
        JsonValue inst;
        pair.second->Serialize(inst);
        instances.SetField(pair.first, inst);
    }
    root.SetField("mapInstances", instances);

    JsonUtils::Write(root, out);
}

void WorldManager::Deserialize(const std::string& in) {
    JsonValue root;
    if (!JsonUtils::Parse(in, root)) return;

    currentMapId = root.HasField("currentMapId") ? root["currentMapId"].AsString() : "";
    currentRoomId = root.HasField("currentRoomId") ? root["currentRoomId"].AsString() : "";

    globalFlags.clear();
    if (root.HasField("globalFlags")) {
        const JsonValue& flags = root["globalFlags"];
        for (const auto& key : flags.GetObjectKeys()) {
            globalFlags[key] = flags[key].AsInt();
        }
    }

    globalEntityStates.clear();
    if (root.HasField("globalEntityStates")) {
        const JsonValue& gents = root["globalEntityStates"];
        for (const auto& key : gents.GetObjectKeys()) {
            globalEntityStates[key] = gents[key].AsString();
        }
    }

    mapInstances.clear();
    if (root.HasField("mapInstances")) {
        const JsonValue& insts = root["mapInstances"];
        for (const auto& key : insts.GetObjectKeys()) {
            auto inst = std::make_unique<MapInstance>(key);
            inst->Deserialize(insts[key]);
            mapInstances[key] = std::move(inst);
        }
    }

    currentPipeline.clear();
    pipelineIndex = 0;
    waitingForEvent = false;
}

void WorldManager::ExecuteRoomPipeline() {
    const RoomDef* room = GetCurrentRoomDef();
    if (!room) {
        waitingForEvent = false;
        return;
    }
    currentPipeline = room->onEnterPipeline;
    pipelineIndex = 0;
    waitingForEvent = false;
    AdvancePipeline();
}

void WorldManager::AdvancePipeline() {
    if (waitingForEvent) return;

    while (pipelineIndex < currentPipeline.size()) {
        const PipelineStep& step = currentPipeline[pipelineIndex];
        pipelineIndex++;

        if (step.type == "DIALOGUE") {
            pendingEvent = WorldEvent{WorldEventType::RequestDialogue, step.param1, 0, ""};
            waitingForEvent = true;
            if (eventCallback) eventCallback(pendingEvent);
            return;
        } else if (step.type == "BATTLE") {
            pendingEvent = WorldEvent{WorldEventType::RequestBattle, step.param1, step.param2, ""};
            waitingForEvent = true;
            if (eventCallback) eventCallback(pendingEvent);
            return;
        } else if (step.type == "LOOT") {
            pendingEvent = WorldEvent{WorldEventType::RequestLoot, step.param1, step.param2, ""};
            waitingForEvent = true;
            if (eventCallback) eventCallback(pendingEvent);
            return;
        } else if (step.type == "SET_FLAG") {
            globalFlags[step.param1] = step.param2;
        } else if (step.type == "CHECK_CONDITION") {
            if (EvaluateCondition(step.param1, step.param2)) {
                if (!step.param3.empty()) {
                    for (size_t i = 0; i < currentPipeline.size(); ++i) {
                        if (currentPipeline[i].type == "LABEL" && currentPipeline[i].param1 == step.param3) {
                            pipelineIndex = i + 1;
                            break;
                        }
                    }
                }
            }
        } else if (step.type == "LABEL") {
            // no-op
        } else if (step.type == "JUMP") {
            if (!step.param1.empty()) {
                for (size_t i = 0; i < currentPipeline.size(); ++i) {
                    if (currentPipeline[i].type == "LABEL" && currentPipeline[i].param1 == step.param1) {
                        pipelineIndex = i + 1;
                        break;
                    }
                }
            }
        } else if (step.type == "NOTIFICATION") {
            pendingEvent = WorldEvent{WorldEventType::Notification, step.param1, 0, ""};
            waitingForEvent = true;
            if (eventCallback) eventCallback(pendingEvent);
            return;
        }
    }

    pendingEvent = WorldEvent{WorldEventType::None, "", 0, ""};
    waitingForEvent = false;
}

bool WorldManager::EvaluateCondition(const std::string& flag, int expectedValue) const {
    auto it = globalFlags.find(flag);
    if (it == globalFlags.end()) return expectedValue == 0;
    return it->second == expectedValue;
}

const RoomDef* WorldManager::GetCurrentRoomDef() const {
    if (!mapData || currentMapId.empty()) return nullptr;
    const MapDef* map = mapData->GetMap(currentMapId);
    if (!map) return nullptr;
    return map->roomGraph.GetRoom(currentRoomId);
}

MapInstance* WorldManager::GetCurrentMapInstance() const {
    auto it = mapInstances.find(currentMapId);
    return (it != mapInstances.end()) ? it->second.get() : nullptr;
}

bool WorldManager::IsEntityProcessed(const std::string& roomId, const std::string& entityId) const {
    if (!mapData || currentMapId.empty()) return false;
    const MapDef* map = mapData->GetMap(currentMapId);
    if (!map) return false;
    const RoomDef* room = map->roomGraph.GetRoom(roomId);
    if (!room) return false;
    for (const auto& entity : room->entities) {
        if (entity.id == entityId) {
            if (entity.isGlobalState) {
                auto it = globalEntityStates.find(entityId);
                return it != globalEntityStates.end() && it->second == "PROCESSED";
            } else {
                MapInstance* inst = GetCurrentMapInstance();
                if (!inst) return false;
                return inst->GetEntityState(roomId, entityId) == "PROCESSED";
            }
        }
    }
    return false;
}
