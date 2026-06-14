#include "WorldFacade.h"
#include "WorldManager.h"
#include "WorldConfigLoader.h"
#include "MapData.h"
#include "WorldEvent.h"
#include <sstream>

class WorldFacade::Impl {
public:
    MapData mapData;
    WorldManager manager;
    WorldEventCallback callback;

    Impl() {
        manager.SetMapData(&mapData);
        manager.SetEventCallback([this](const WorldEvent& e) {
            if (callback) callback(e);
        });
    }
};

WorldFacade::WorldFacade() : impl(std::make_unique<Impl>()) {}
WorldFacade::~WorldFacade() = default;

bool WorldFacade::LoadConfig(const std::string& filepath) {
    return WorldConfigLoader::LoadFromFile(filepath, impl->mapData);
}

bool WorldFacade::LoadConfigFromString(const std::string& content) {
    return WorldConfigLoader::LoadFromString(content, impl->mapData);
}

void WorldFacade::EnterMap(const std::string& mapId, const std::string& entryRoomId) {
    impl->manager.EnterMap(mapId, entryRoomId);
}

void WorldFacade::LeaveMap() {
    impl->manager.LeaveMap();
}

bool WorldFacade::MoveToRoom(int exitIndex) {
    return impl->manager.MoveToRoom(exitIndex);
}

bool WorldFacade::InteractWithEntity(int entityIndex) {
    return impl->manager.InteractWithEntity(entityIndex);
}

bool WorldFacade::HasPendingEvent() const {
    return impl->manager.HasPendingEvent();
}

WorldEvent WorldFacade::GetPendingEvent() const {
    return impl->manager.GetPendingEvent();
}

void WorldFacade::NotifyEventComplete(bool success) {
    impl->manager.NotifyEventComplete(success);
}

void WorldFacade::SetEventCallback(WorldEventCallback cb) {
    impl->callback = std::move(cb);
}

WorldFacade::RoomDisplayInfo WorldFacade::GetCurrentRoom() const {
    RoomDisplayInfo info;
    auto room = impl->manager.GetCurrentRoomInfo();
    info.name = room.name;
    info.description = room.description;
    return info;
}

std::vector<WorldFacade::ExitDisplayInfo> WorldFacade::GetAvailableExits() const {
    std::vector<ExitDisplayInfo> result;
    auto exits = impl->manager.GetAvailableExits();
    for (const auto& exit : exits) {
        ExitDisplayInfo info;
        info.description = exit.description;
        info.isAvailable = exit.isAvailable;
        std::string targetMap = exit.targetMapId.empty() ? impl->manager.GetCurrentMapId() : exit.targetMapId;
        const MapDef* map = impl->mapData.GetMap(targetMap);
        if (map) {
            const RoomDef* room = map->roomGraph.GetRoom(exit.targetRoomId);
            info.targetRoomName = room ? room->name : exit.targetRoomId;
        } else {
            info.targetRoomName = exit.targetRoomId;
        }
        result.push_back(info);
    }
    return result;
}

std::vector<WorldFacade::EntityDisplayInfo> WorldFacade::GetRoomEntities() const {
    std::vector<EntityDisplayInfo> result;
    auto entities = impl->manager.GetRoomEntities();
    for (const auto& e : entities) {
        EntityDisplayInfo info;
        info.id = e.id;
        info.name = e.name;
        info.description = e.description;
        info.isInteractable = e.isInteractable;
        switch (e.type) {
            case EntityType::NPC: info.typeName = "NPC"; break;
            case EntityType::Chest: info.typeName = "Chest"; break;
            case EntityType::Monster: info.typeName = "Monster"; break;
            case EntityType::Portal: info.typeName = "Portal"; break;
            case EntityType::Trigger: info.typeName = "Trigger"; break;
            case EntityType::Interactive: info.typeName = "Interactive"; break;
            default: info.typeName = "Unknown"; break;
        }
        result.push_back(info);
    }
    return result;
}

void WorldFacade::MarkEntityProcessed(const std::string& entityId) {
    impl->manager.MarkEntityProcessed(entityId);
}

void WorldFacade::SetGlobalFlag(const std::string& flag, int value) {
    impl->manager.SetGlobalFlag(flag, value);
}

int WorldFacade::GetGlobalFlag(const std::string& flag) const {
    return impl->manager.GetGlobalFlag(flag);
}

void WorldFacade::Serialize(std::string& out) const {
    impl->manager.Serialize(out);
}

void WorldFacade::Deserialize(const std::string& in) {
    impl->manager.Deserialize(in);
}

bool WorldFacade::IsInMap() const {
    return !impl->manager.GetCurrentMapId().empty();
}

std::string WorldFacade::GetCurrentMapName() const {
    const MapDef* map = impl->mapData.GetMap(impl->manager.GetCurrentMapId());
    return map ? map->name : "";
}

bool WorldFacade::IsEntityProcessed(const std::string& entityId) const {
    if (!IsInMap()) return false;
    const MapDef* map = impl->mapData.GetMap(impl->manager.GetCurrentMapId());
    if (!map) return false;
    for (const auto& roomId : map->roomGraph.GetAllRoomIds()) {
        const RoomDef* room = map->roomGraph.GetRoom(roomId);
        if (!room) continue;
        for (const auto& entity : room->entities) {
            if (entity.id == entityId)
                return impl->manager.IsEntityProcessed(roomId, entityId);
        }
    }
    return false;
}

std::vector<std::pair<std::string, int>> WorldFacade::GetAllGlobalFlags() const {
    std::vector<std::pair<std::string, int>> result;
    for (const auto& p : impl->manager.GetAllGlobalFlags())
        result.emplace_back(p.first, p.second);
    return result;
}

std::string WorldFacade::GetDebugWorldState() const {
    std::ostringstream oss;
    oss << "========== 全地图状态 ==========\n";
    if (IsInMap())
        oss << "[当前位置] " << GetCurrentMapName() << " - " << GetCurrentRoom().name << "\n";
    else
        oss << "[当前位置] 未在地图中\n";
    oss << "\n";

    for (const auto& mapId : impl->mapData.GetAllMapIds()) {
        const MapDef* map = impl->mapData.GetMap(mapId);
        if (!map) continue;
        oss << "--- 地图: " << map->name << " (" << mapId << ") ---\n";

        for (const auto& roomId : map->roomGraph.GetAllRoomIds()) {
            const RoomDef* room = map->roomGraph.GetRoom(roomId);
            if (!room) continue;

            bool cur = IsInMap() && impl->manager.GetCurrentMapId() == mapId
                                 && impl->manager.GetCurrentRoomId() == roomId;
            oss << "  [房间] " << room->name << " (" << roomId << ")"
                << (cur ? " [★当前]" : "") << "\n";

            if (!room->entities.empty()) {
                oss << "    实体:\n";
                for (const auto& e : room->entities) {
                    bool done = impl->manager.IsEntityProcessed(roomId, e.id);
                    const char* t = "Unknown";
                    switch (e.type) {
                        case EntityType::NPC: t = "NPC"; break;
                        case EntityType::Chest: t = "Chest"; break;
                        case EntityType::Monster: t = "Monster"; break;
                        case EntityType::Portal: t = "Portal"; break;
                        case EntityType::Trigger: t = "Trigger"; break;
                        case EntityType::Interactive: t = "Interactive"; break;
                    }
                    oss << "      - " << e.name << " [" << t << "] "
                        << (e.isGlobalState ? "(全局) " : "")
                        << (done ? "[已处理]" : "[可交互]") << "\n";
                }
            } else {
                oss << "    实体: (无)\n";
            }

            if (!room->exits.empty()) {
                oss << "    出口:\n";
                for (const auto& ex : room->exits) {
                    const RoomDef* tgt = map->roomGraph.GetRoom(ex.targetRoomId);
                    oss << "      -> " << (tgt ? tgt->name : ex.targetRoomId);
                    if (!ex.conditionFlag.empty())
                        oss << (GetGlobalFlag(ex.conditionFlag) ? " (已解锁)" :
                                " (未解锁, 需:" + ex.conditionFlag + ")");
                    else
                        oss << " (可用)";
                    oss << "\n";
                }
            } else {
                oss << "    出口: (无)\n";
            }
        }
        oss << "\n";
    }

    auto flags = GetAllGlobalFlags();
    if (!flags.empty()) {
        oss << "[全局标记]\n";
        for (const auto& p : flags) oss << "  " << p.first << " = " << p.second << "\n";
    } else {
        oss << "[全局标记] (无)\n";
    }
    return oss.str();
}
