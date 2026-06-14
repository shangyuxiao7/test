#include "RoomGraph.h"

void RoomGraph::Clear() { rooms.clear(); }
void RoomGraph::AddRoom(const RoomDef& room) { rooms[room.id] = room; }
const RoomDef* RoomGraph::GetRoom(const std::string& id) const {
    auto it = rooms.find(id);
    return (it != rooms.end()) ? &it->second : nullptr;
}
bool RoomGraph::HasRoom(const std::string& id) const { return rooms.find(id) != rooms.end(); }
std::vector<std::string> RoomGraph::GetAllRoomIds() const {
    std::vector<std::string> result;
    for (const auto& pair : rooms) result.push_back(pair.first);
    return result;
}
