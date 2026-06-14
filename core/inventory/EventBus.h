#pragma once
#include <string>
#include <functional>
#include <vector>

// 事件类型枚举（部分，背包系统只关心这些）
enum class InventoryEventType {
    ItemAdded,          // 获得物品
    ItemRemoved,        // 失去物品
    ItemUsed,           // 使用物品
    ItemEquipped,       // 装备物品
    GoldChanged,        // 金钱变化
    InventoryFull       // 背包已满
};

// 事件数据结构（简化版）
struct InventoryGameEvent {
    InventoryEventType type;
    std::string itemId;
    std::string characterId;    // 目标角色ID（使用/装备时）
    int count = 0;
    int value = 0;              // 金钱变化数值等
};

// 事件总线（全局单例）
class EventBus {
public:
    static EventBus& Instance();
    void Publish(const InventoryGameEvent& e);

    // 测试辅助：获取最后发布的事件
    std::vector<InventoryGameEvent> GetHistory() const { return history; }
    void ClearHistory() { history.clear(); }

private:
    std::vector<InventoryGameEvent> history;
};
