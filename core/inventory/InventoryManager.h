#pragma once
#include "Item.h"
#include "../../foundation/ISerializable.h"
#include <string>
#include <vector>
#include <map>

class InventoryManager : public ISerializable {
public:
    InventoryManager(int capacity = 20);
    ~InventoryManager() = default;

    // ---------- 查询 ----------

    // 获取所有物品（按 itemId 字典序）
    std::vector<ItemStack> GetItems() const;

    // 获取特定物品的数量
    int GetItemCount(const std::string& itemId) const;

    // 检查是否有某物品
    bool HasItem(const std::string& itemId, int count = 1) const;

    // 获取当前物品种类数
    int GetItemSlotCount() const;

    // 获取背包容量
    int GetCapacity() const;

    // 检查背包是否已满（按物品种类数计算，不是按总数量）
    bool IsFull() const;

    // 获取金钱
    int GetGold() const;

    // ---------- 修改 ----------

    // 添加物品，返回是否成功
    // 失败原因：找不到定义、超过单物品堆叠上限、背包已满（新物品种类）
    bool AddItem(const std::string& itemId, int count = 1);

    // 移除物品，返回是否成功
    // 失败原因：数量不足
    bool RemoveItem(const std::string& itemId, int count = 1);

    // 使用物品（消耗品）
    // 职责：扣除数量，发出 ItemUsed 事件，不执行实际效果
    // 效果由外部系统（如角色系统）监听事件后执行
    bool UseItem(const std::string& itemId, int count = 1, const std::string& targetCharacterId = "");

    // 装备物品
    // 职责：验证是否为装备、扣除数量，发出 ItemEquipped 事件
    // 实际装备效果（属性加成）由外部系统处理
    bool EquipItem(const std::string& itemId, const std::string& characterId);

    // 丢弃物品
    bool DiscardItem(const std::string& itemId, int count = 1);

    // 添加金钱
    void AddGold(int amount);

    // 扣除金钱，返回是否成功
    bool SpendGold(int amount);

    // ---------- 存档接口 ----------

    void Serialize(std::string& out) const override;
    void Deserialize(const std::string& in) override;

private:
    int capacity;
    int gold = 0;
    std::map<std::string, int> items;   // itemId -> count

    // 辅助：检查添加是否合法
    bool CanAdd(const std::string& itemId, int count) const;
};
