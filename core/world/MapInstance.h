#pragma once
#include <string>
#include <unordered_map>
#include "../../foundation/JsonValue.h"

class MapInstance {
public:
    explicit MapInstance(const std::string& mapId);

    void SetEntityState(const std::string& roomId, const std::string& entityId, const std::string& state);
    std::string GetEntityState(const std::string& roomId, const std::string& entityId) const;
    bool IsEntityDefeated(const std::string& roomId, const std::string& entityId) const;
    void MarkEntityDefeated(const std::string& roomId, const std::string& entityId);

    const std::string& GetMapId() const;

    void Serialize(JsonValue& out) const;
    void Deserialize(const JsonValue& in);

private:
    std::string mapId;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> entityStates;
};
