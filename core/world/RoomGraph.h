#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Entity.h"

struct ExitDef {
    std::string targetRoomId;
    std::string targetMapId;  // 空表示同地图
    std::string description;
    std::string conditionFlag;
};

struct PipelineStep {
    std::string type;
    std::string param1;
    int param2 = 0;
    std::string param3;
};

struct RoomDef {
    std::string id;
    std::string name;
    std::string description;
    std::vector<ExitDef> exits;
    std::vector<EntityDef> entities;
    std::vector<PipelineStep> onEnterPipeline;
};

class RoomGraph {
public:
    void Clear();
    void AddRoom(const RoomDef& room);
    const RoomDef* GetRoom(const std::string& id) const;
    bool HasRoom(const std::string& id) const;
    std::vector<std::string> GetAllRoomIds() const;

private:
    std::unordered_map<std::string, RoomDef> rooms;
};
