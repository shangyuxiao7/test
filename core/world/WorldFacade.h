#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "WorldEvent.h"

class WorldFacade {
public:
    explicit WorldFacade();
    ~WorldFacade();
    WorldFacade(const WorldFacade&) = delete;
    WorldFacade& operator=(const WorldFacade&) = delete;

    bool LoadConfig(const std::string& filepath);
    bool LoadConfigFromString(const std::string& jsonContent);

    void EnterMap(const std::string& mapId, const std::string& entryRoomId);
    void LeaveMap();

    bool MoveToRoom(int exitIndex);
    bool InteractWithEntity(int entityIndex);

    bool HasPendingEvent() const;
    WorldEvent GetPendingEvent() const;
    void NotifyEventComplete(bool success);

    using WorldEventCallback = std::function<void(const WorldEvent&)>;
    void SetEventCallback(WorldEventCallback cb);

    struct RoomDisplayInfo {
        std::string name;
        std::string description;
    };
    struct ExitDisplayInfo {
        std::string description;
        std::string targetRoomName;
        bool isAvailable;
    };
    struct EntityDisplayInfo {
        std::string id;
        std::string name;
        std::string description;
        std::string typeName;
        bool isInteractable;
    };

    RoomDisplayInfo GetCurrentRoom() const;
    std::vector<ExitDisplayInfo> GetAvailableExits() const;
    std::vector<EntityDisplayInfo> GetRoomEntities() const;

    void MarkEntityProcessed(const std::string& entityId);
    void SetGlobalFlag(const std::string& flag, int value);
    int GetGlobalFlag(const std::string& flag) const;

    void Serialize(std::string& out) const;
    void Deserialize(const std::string& in);

    bool IsInMap() const;
    std::string GetCurrentMapName() const;

    bool IsEntityProcessed(const std::string& entityId) const;
    std::vector<std::pair<std::string, int>> GetAllGlobalFlags() const;
    std::string GetDebugWorldState() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
