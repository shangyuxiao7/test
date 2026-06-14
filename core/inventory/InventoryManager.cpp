#include "InventoryManager.h"
#include "ItemDatabase.h"
#include "EventBus.h"
#include <sstream>

InventoryManager::InventoryManager(int capacity) : capacity(capacity) {}

std::vector<ItemStack> InventoryManager::GetItems() const {
    std::vector<ItemStack> result;
    for (const auto& pair : items) {
        if (pair.second > 0) {
            ItemStack stack;
            stack.itemId = pair.first;
            stack.count = pair.second;
            result.push_back(stack);
        }
    }
    return result;
}

int InventoryManager::GetItemCount(const std::string& itemId) const {
    auto it = items.find(itemId);
    return it != items.end() ? it->second : 0;
}

bool InventoryManager::HasItem(const std::string& itemId, int count) const {
    return GetItemCount(itemId) >= count;
}

int InventoryManager::GetItemSlotCount() const {
    return static_cast<int>(items.size());
}

int InventoryManager::GetCapacity() const {
    return capacity;
}

bool InventoryManager::IsFull() const {
    return GetItemSlotCount() >= capacity;
}

int InventoryManager::GetGold() const {
    return gold;
}

bool InventoryManager::CanAdd(const std::string& itemId, int count) const {
    const ItemDef* def = ItemDatabase::GetDef(itemId);
    if (!def) return false;
    if (count <= 0) return false;

    int currentCount = GetItemCount(itemId);
    if (currentCount > 0) {
        // 已有该物品，检查堆叠上限
        return currentCount + count <= def->maxStack;
    } else {
        // 新物品，检查背包容量
        if (IsFull()) return false;
        return count <= def->maxStack;
    }
}

bool InventoryManager::AddItem(const std::string& itemId, int count) {
    if (!CanAdd(itemId, count)) return false;

    items[itemId] += count;

    InventoryGameEvent event;
    event.type = InventoryEventType::ItemAdded;
    event.itemId = itemId;
    event.count = count;
    EventBus::Instance().Publish(event);

    return true;
}

bool InventoryManager::RemoveItem(const std::string& itemId, int count) {
    if (!HasItem(itemId, count)) return false;

    items[itemId] -= count;
    if (items[itemId] <= 0) {
        items.erase(itemId);
    }

    InventoryGameEvent event;
    event.type = InventoryEventType::ItemRemoved;
    event.itemId = itemId;
    event.count = count;
    EventBus::Instance().Publish(event);

    return true;
}

bool InventoryManager::UseItem(const std::string& itemId, int count, const std::string& targetCharacterId) {
    const ItemDef* def = ItemDatabase::GetDef(itemId);
    if (!def || def->type != ItemType::Consumable) return false;
    if (!HasItem(itemId, count)) return false;

    items[itemId] -= count;
    if (items[itemId] <= 0) {
        items.erase(itemId);
    }

    InventoryGameEvent event;
    event.type = InventoryEventType::ItemUsed;
    event.itemId = itemId;
    event.characterId = targetCharacterId;
    event.count = count;
    EventBus::Instance().Publish(event);

    return true;
}

bool InventoryManager::EquipItem(const std::string& itemId, const std::string& characterId) {
    const ItemDef* def = ItemDatabase::GetDef(itemId);
    if (!def || def->type != ItemType::Equipment) return false;
    if (!HasItem(itemId, 1)) return false;

    items[itemId] -= 1;
    if (items[itemId] <= 0) {
        items.erase(itemId);
    }

    InventoryGameEvent event;
    event.type = InventoryEventType::ItemEquipped;
    event.itemId = itemId;
    event.characterId = characterId;
    event.count = 1;
    EventBus::Instance().Publish(event);

    return true;
}

bool InventoryManager::DiscardItem(const std::string& itemId, int count) {
    return RemoveItem(itemId, count);
}

void InventoryManager::AddGold(int amount) {
    if (amount <= 0) return;
    gold += amount;

    InventoryGameEvent event;
    event.type = InventoryEventType::GoldChanged;
    event.value = amount;
    EventBus::Instance().Publish(event);
}

bool InventoryManager::SpendGold(int amount) {
    if (amount <= 0) return true;
    if (gold < amount) return false;
    gold -= amount;

    InventoryGameEvent event;
    event.type = InventoryEventType::GoldChanged;
    event.value = -amount;
    EventBus::Instance().Publish(event);

    return true;
}

void InventoryManager::Serialize(std::string& out) const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"gold\":" << gold << ",";
    oss << "\"capacity\":" << capacity << ",";
    oss << "\"items\":{";
    bool first = true;
    for (const auto& pair : items) {
        if (!first) oss << ",";
        oss << "\"" << pair.first << "\":" << pair.second;
        first = false;
    }
    oss << "}";
    oss << "}";
    out = oss.str();
}

void InventoryManager::Deserialize(const std::string& in) {
    items.clear();
    gold = 0;
    capacity = 20;

    // 辅助：解析整数字段
    auto parseIntField = [&](const std::string& fieldName, int& outVal) {
        std::string key = "\"" + fieldName + "\":";
        size_t pos = in.find(key);
        if (pos == std::string::npos) return;
        pos += key.length();
        while (pos < in.size() && (in[pos] == ' ' || in[pos] == '\t')) ++pos;
        size_t end = pos;
        while (end < in.size() && (in[end] == '-' || (in[end] >= '0' && in[end] <= '9'))) ++end;
        if (end > pos) {
            outVal = std::stoi(in.substr(pos, end - pos));
        }
    };

    parseIntField("gold", gold);
    parseIntField("capacity", capacity);

    // 解析 items 对象
    std::string itemsKey = "\"items\":";
    size_t itemsPos = in.find(itemsKey);
    if (itemsPos == std::string::npos) return;

    size_t braceOpen = in.find('{', itemsPos + itemsKey.length());
    if (braceOpen == std::string::npos) return;

    int braceCount = 1;
    size_t braceClose = braceOpen + 1;
    while (braceClose < in.size() && braceCount > 0) {
        if (in[braceClose] == '{') ++braceCount;
        else if (in[braceClose] == '}') --braceCount;
        ++braceClose;
    }
    if (braceCount != 0) return;
    --braceClose;

    std::string itemsContent = in.substr(braceOpen + 1, braceClose - braceOpen - 1);

    size_t pos = 0;
    while (pos < itemsContent.size()) {
        size_t q1 = itemsContent.find('"', pos);
        if (q1 == std::string::npos) break;
        size_t q2 = itemsContent.find('"', q1 + 1);
        if (q2 == std::string::npos) break;
        std::string key = itemsContent.substr(q1 + 1, q2 - q1 - 1);

        size_t colon = itemsContent.find(':', q2);
        if (colon == std::string::npos) break;

        size_t valStart = colon + 1;
        while (valStart < itemsContent.size() && (itemsContent[valStart] == ' ' || itemsContent[valStart] == '\t')) ++valStart;

        size_t valEnd = valStart;
        while (valEnd < itemsContent.size() && itemsContent[valEnd] >= '0' && itemsContent[valEnd] <= '9') ++valEnd;

        if (valEnd > valStart) {
            int value = std::stoi(itemsContent.substr(valStart, valEnd - valStart));
            items[key] = value;
        }

        pos = valEnd;
        size_t comma = itemsContent.find(',', pos);
        if (comma != std::string::npos) pos = comma + 1;
        else break;
    }
}
