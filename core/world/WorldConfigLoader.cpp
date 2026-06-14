#include "WorldConfigLoader.h"
#include "../../foundation/ConfigLoader.h"
#include "../../foundation/JsonValue.h"
#include "../../foundation/JsonUtils.h"
#include "MapData.h"
#include "RoomGraph.h"
#include "Entity.h"

static bool ParseEntityDef(const JsonValue& node, EntityDef& out) {
    if (!node.HasField("id") || !node.HasField("name")) return false;
    out.id = node["id"].AsString();
    out.name = node["name"].AsString();
    out.description = node.HasField("description") ? node["description"].AsString() : "";
    std::string typeStr = node.HasField("type") ? node["type"].AsString() : "NPC";
    if (typeStr == "NPC") out.type = EntityType::NPC;
    else if (typeStr == "Chest") out.type = EntityType::Chest;
    else if (typeStr == "Monster") out.type = EntityType::Monster;
    else if (typeStr == "Portal") out.type = EntityType::Portal;
    else if (typeStr == "Trigger") out.type = EntityType::Trigger;
    else out.type = EntityType::Interactive;
    out.eventId = node.HasField("eventId") ? node["eventId"].AsString() : "";
    out.isGlobalState = node.HasField("isGlobalState") ? node["isGlobalState"].AsBool() : false;
    out.targetMapId = node.HasField("targetMapId") ? node["targetMapId"].AsString() : "";
    out.targetRoomId = node.HasField("targetRoomId") ? node["targetRoomId"].AsString() : "";
    return true;
}

static bool ParseExitDef(const JsonValue& node, ExitDef& out) {
    if (!node.HasField("targetRoomId") || !node.HasField("description")) return false;
    out.targetRoomId = node["targetRoomId"].AsString();
    out.targetMapId = node.HasField("targetMapId") ? node["targetMapId"].AsString() : "";
    out.description = node["description"].AsString();
    out.conditionFlag = node.HasField("conditionFlag") ? node["conditionFlag"].AsString() : "";
    return true;
}

static bool ParsePipelineStep(const JsonValue& node, PipelineStep& out) {
    if (!node.HasField("type")) return false;
    out.type = node["type"].AsString();
    out.param1 = node.HasField("param1") ? node["param1"].AsString() : "";
    out.param2 = node.HasField("param2") ? node["param2"].AsInt() : 0;
    out.param3 = node.HasField("param3") ? node["param3"].AsString() : "";
    return true;
}

static bool ParseRoomDef(const JsonValue& node, RoomDef& out) {
    if (!node.HasField("id") || !node.HasField("name")) return false;
    out.id = node["id"].AsString();
    out.name = node["name"].AsString();
    out.description = node.HasField("description") ? node["description"].AsString() : "";
    if (node.HasField("exits")) {
        const JsonValue& arr = node["exits"];
        for (size_t i = 0; i < arr.ArraySize(); ++i) {
            ExitDef exit;
            if (ParseExitDef(arr[i], exit)) out.exits.push_back(exit);
        }
    }
    if (node.HasField("entities")) {
        const JsonValue& arr = node["entities"];
        for (size_t i = 0; i < arr.ArraySize(); ++i) {
            EntityDef entity;
            if (ParseEntityDef(arr[i], entity)) out.entities.push_back(entity);
        }
    }
    if (node.HasField("onEnterPipeline")) {
        const JsonValue& arr = node["onEnterPipeline"];
        for (size_t i = 0; i < arr.ArraySize(); ++i) {
            PipelineStep step;
            if (ParsePipelineStep(arr[i], step)) out.onEnterPipeline.push_back(step);
        }
    }
    return true;
}

static bool ParseMapDef(const JsonValue& node, MapDef& out) {
    if (!node.HasField("id") || !node.HasField("name")) return false;
    out.id = node["id"].AsString();
    out.name = node["name"].AsString();
    if (node.HasField("rooms")) {
        const JsonValue& arr = node["rooms"];
        for (size_t i = 0; i < arr.ArraySize(); ++i) {
            RoomDef room;
            if (ParseRoomDef(arr[i], room)) out.roomGraph.AddRoom(room);
        }
    }
    return true;
}

static bool ParseMapData(const JsonValue& root, MapData& out) {
    out.Clear();
    if (!root.HasField("maps")) return false;
    const JsonValue& mapsArr = root["maps"];
    for (size_t i = 0; i < mapsArr.ArraySize(); ++i) {
        MapDef map;
        if (ParseMapDef(mapsArr[i], map)) out.RegisterMap(map);
    }
    return true;
}

bool WorldConfigLoader::LoadFromFile(const std::string& filepath, MapData& out) {
    JsonValue root;
    if (!ConfigLoader::LoadFromFile(filepath, root)) return false;
    return ParseMapData(root, out);
}

bool WorldConfigLoader::LoadFromString(const std::string& content, MapData& out) {
    JsonValue root;
    if (!ConfigLoader::LoadFromString(content, root)) return false;
    return ParseMapData(root, out);
}
