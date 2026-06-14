#include "MapInstance.h"

MapInstance::MapInstance(const std::string& id) : mapId(id) {}

void MapInstance::SetEntityState(const std::string& roomId, const std::string& entityId, const std::string& state) {
    entityStates[roomId][entityId] = state;
}

std::string MapInstance::GetEntityState(const std::string& roomId, const std::string& entityId) const {
    auto rit = entityStates.find(roomId);
    if (rit == entityStates.end()) return "";
    auto eit = rit->second.find(entityId);
    return (eit != rit->second.end()) ? eit->second : "";
}

bool MapInstance::IsEntityDefeated(const std::string& roomId, const std::string& entityId) const {
    return GetEntityState(roomId, entityId) == "PROCESSED";
}

void MapInstance::MarkEntityDefeated(const std::string& roomId, const std::string& entityId) {
    SetEntityState(roomId, entityId, "PROCESSED");
}

const std::string& MapInstance::GetMapId() const { return mapId; }

void MapInstance::Serialize(JsonValue& out) const {
    out = JsonValue(JsonType::Object);
    out.SetField("mapId", JsonValue(mapId));
    JsonValue rooms(JsonType::Object);
    for (const auto& pair : entityStates) {
        JsonValue roomNode(JsonType::Object);
        for (const auto& epair : pair.second) {
            roomNode.SetField(epair.first, JsonValue(epair.second));
        }
        rooms.SetField(pair.first, roomNode);
    }
    out.SetField("entityStates", rooms);
}

void MapInstance::Deserialize(const JsonValue& in) {
    mapId = in["mapId"].AsString();
    entityStates.clear();
    const JsonValue& rooms = in["entityStates"];
    for (const auto& roomId : rooms.GetObjectKeys()) {
        const JsonValue& roomNode = rooms[roomId];
        for (const auto& entityId : roomNode.GetObjectKeys()) {
            entityStates[roomId][entityId] = roomNode[entityId].AsString();
        }
    }
}
