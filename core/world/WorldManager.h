#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "../../foundation/ISerializable.h"
#include "WorldEvent.h"
#include "RoomGraph.h"
#include "MapData.h"
#include "MapInstance.h"

class WorldManager : public ISerializable {
public:
    using WorldEventCallback = std::function<void(const WorldEvent&)>;

    WorldManager();

    void SetMapData(MapData* data);
    void SetEventCallback(WorldEventCallback cb);

    void EnterMap(const std::string& mapId, const std::string& entryRoomId);
    void LeaveMap();

    bool MoveToRoom(int exitIndex);
    bool InteractWithEntity(int entityIndex);

    bool HasPendingEvent() const;
    WorldEvent GetPendingEvent() const;
    void NotifyEventComplete(bool success);

    struct RoomInfo {
        std::string id;
        std::string name;
        std::string description;
    };
    struct ExitInfo {
        std::string targetRoomId;
        std::string targetMapId;
        std::string description;
        bool isAvailable;
    };
    struct EntityInfo {
        std::string id;
        std::string name;
        std::string description;
        EntityType type;
        bool isInteractable;
    };

    RoomInfo GetCurrentRoomInfo() const;
    std::vector<ExitInfo> GetAvailableExits() const;
    std::vector<EntityInfo> GetRoomEntities() const;

    void MarkEntityProcessed(const std::string& entityId);
    void SetGlobalFlag(const std::string& flag, int value);
    int GetGlobalFlag(const std::string& flag) const;

    std::string GetCurrentMapId() const;
    std::string GetCurrentRoomId() const;
    bool IsEntityProcessed(const std::string& roomId, const std::string& entityId) const;
    const std::unordered_map<std::string, int>& GetAllGlobalFlags() const { return globalFlags; }

    void Serialize(std::string& out) const override;
    void Deserialize(const std::string& in) override;

private:
    MapData* mapData = nullptr;
    std::unordered_map<std::string, std::unique_ptr<MapInstance>> mapInstances;
    std::string currentMapId;
    std::string currentRoomId;

    std::unordered_map<std::string, int> globalFlags;
    std::unordered_map<std::string, std::string> globalEntityStates;

    std::vector<PipelineStep> currentPipeline;
    size_t pipelineIndex = 0;
    bool waitingForEvent = false;
    WorldEvent pendingEvent;
    WorldEventCallback eventCallback;

    MapInstance* GetCurrentMapInstance() const;
    void ExecuteRoomPipeline();
    void AdvancePipeline();
    bool EvaluateCondition(const std::string& flag, int expectedValue) const;
    const RoomDef* GetCurrentRoomDef() const;
};
