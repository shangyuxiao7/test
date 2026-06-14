# 背包模块接口文档 (Inventory Module)

> **对外接口类**: `InventoryManager`（唯一暴露类）  
> **配套 UI**: `GameConsoleUI::ShowInventory`（`core/ui/GameConsoleUI.h`）  
> **模块路径**: `core/inventory/`  
> **配置路径**: `assets/config/items.json`

---

## 一、模块概述

格子制堆叠存储系统。职责边界：**只负责存储、计数、发事件，不执行任何实际效果**。

- 添加/移除/使用/装备/丢弃物品
- 金钱系统
- 按物品种类数计算容量（非总数量）
- 使用/装备时通过 `EventBus` 发布事件，由外部系统（角色/战斗）监听并执行实际效果

---

## 二、依赖关系树状图

```
                              main / GameManager
                                     │
                    ┌────────────────┼────────────────┐
                    │                │                │
              GameConsoleUI   InventoryManager（唯一对外接口）  PartyManager
                    │                │                │
             ShowInventory      ItemStack         Equip/Use
                    │                │                │
                    │          ItemDataTables         │
                    │                │                │
                    │        ItemConfigLoader         │
                    │                │                │
                    │   assets/config/items.json      │
                    │                                 │
                    └──────────────┬──────────────────┘
                                   │
                             EventBus（全局单例）
                                   │
                    ┌──────────────┘
                    │
              外部监听者（角色系统 / UI）
```

---

## 三、配置加载

```cpp
#include "core/inventory/ItemConfigLoader.h"

ItemConfigLoader::LoadFromFile("assets/config/items.json");
```

| 类 | 方法 | 说明 |
|---|---|---|
| `ItemConfigLoader` | `static bool LoadFromFile(const std::string& filepath)` | 从 JSON 加载物品定义到全局 `ItemDataTables` |
| `ItemConfigLoader` | `static bool LoadFromString(const std::string& jsonContent)` | 从字符串加载（测试用） |

---

## 四、数据结构

### 4.1 `ItemType`（物品类型枚举）

| 值 | 说明 |
|---|---|
| `Consumable` | 消耗品（药水、食物） |
| `Equipment` | 装备（武器、防具、鞋子、饰品） |
| `Material` | 材料（锻造用） |
| `Quest` | 任务道具 |

### 4.2 `EquipmentSlot`（装备槽位枚举）

| 值 | 说明 |
|---|---|
| `Weapon` | 武器 |
| `Armor` | 防具 |
| `Shoes` | 鞋子 |
| `Accessory` | 饰品/宝物 |

### 4.3 `ItemDef`（物品定义，只读配置数据）

| 字段 | 类型 | 说明 |
|---|---|---|
| `id` | `std::string` | 配置表中的唯一ID |
| `name` | `std::string` | 显示名称 |
| `description` | `std::string` | 描述文本 |
| `type` | `ItemType` | 物品类型 |
| `maxStack` | `int` | 最大堆叠数量（默认99） |
| `buyPrice` | `int` | 买入价格 |
| `sellPrice` | `int` | 卖出价格 |
| `equipSlot` | `EquipmentSlot` | 装备槽位（仅 Equipment 类型有效） |
| `bonusAttack` | `int` | 装备攻击加成 |
| `bonusDefense` | `int` | 装备防御加成 |
| `bonusHp` | `int` | 装备生命加成 |
| `bonusSpeed` | `int` | 装备速度加成 |
| `effectScript` | `std::string` | 消耗品效果脚本（如 `"heal_hp:50"`，外部解析执行） |

### 4.4 `ItemStack`（背包中的物品堆叠）

| 字段/方法 | 类型 | 说明 |
|---|---|---|
| `itemId` | `std::string` | 配置表中的物品ID |
| `count` | `int` | 当前持有数量 |
| `GetName() const` | `std::string` | 查询显示名称（委托到 `ItemDataTables`） |
| `GetType() const` | `ItemType` | 查询类型 |
| `GetMaxStack() const` | `int` | 查询最大堆叠数 |
| `IsEquipment() const` | `bool` | 是否为装备 |

### 4.5 `ItemDataTables`（全局配置查询单例）

```cpp
ItemDataTables::Instance().GetItemDef("heal_herb");   // 返回 const ItemDef* 或 nullptr
ItemDataTables::Instance().GetAllItemDefs();          // 返回所有定义
ItemDataTables::Instance().Clear();                   // 清空（测试用）
```

---

## 五、核心接口类：`InventoryManager`

### 5.1 构造

```cpp
#include "core/inventory/InventoryManager.h"

InventoryManager inv(20);   // 容量20格（按物品种类数）
```

| 方法 | 说明 |
|---|---|
| `InventoryManager(int capacity = 20)` | 构造，capacity 为最大物品种类数 |
| `~InventoryManager() = default` | 默认析构 |

### 5.2 查询接口

| 方法 | 返回值 | 说明 |
|---|---|---|
| `std::vector<ItemStack> GetItems() const` | 所有物品（按 `itemId` 字典序） | 获取背包全部内容 |
| `int GetItemCount(const std::string& itemId) const` | 数量 | 获取特定物品持有数 |
| `bool HasItem(const std::string& itemId, int count = 1) const` | bool | 是否持有至少指定数量 |
| `int GetItemSlotCount() const` | 种类数 | 当前占用了多少格 |
| `int GetCapacity() const` | 容量 | 最大可持有种类数 |
| `bool IsFull() const` | bool | 是否已满（新物品种类无法放入） |
| `int GetGold() const` | 金钱数 | 当前持有金钱 |

### 5.3 修改接口

| 方法 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `bool AddItem(const std::string& itemId, int count = 1)` | 物品ID、数量 | bool | 添加物品。失败原因：找不到定义、超过单物品堆叠上限、背包已满（新物品种类） |
| `bool RemoveItem(const std::string& itemId, int count = 1)` | 物品ID、数量 | bool | 移除物品。失败原因：数量不足 |
| `bool UseItem(const std::string& itemId, int count = 1, const std::string& targetCharacterId = "")` | 物品ID、数量、目标角色ID | bool | 使用消耗品。仅扣除数量并发出 `ItemUsed` 事件，**不执行实际效果** |
| `bool EquipItem(const std::string& itemId, const std::string& characterId)` | 物品ID、角色ID | bool | 装备物品。验证是否为装备、扣除数量，发出 `ItemEquipped` 事件，**不执行属性加成** |
| `bool DiscardItem(const std::string& itemId, int count = 1)` | 物品ID、数量 | bool | 丢弃物品 |
| `void AddGold(int amount)` | 金额 | - | 添加金钱 |
| `bool SpendGold(int amount)` | 金额 | bool | 扣除金钱。失败原因：余额不足 |

### 5.4 存档接口

```cpp
std::string saveData;
inv.Serialize(saveData);       // 导出为字符串

InventoryManager inv2(20);
inv2.Deserialize(saveData);    // 从字符串恢复
```

| 方法 | 说明 |
|---|---|
| `void Serialize(std::string& out) const override` | 序列化为字符串 |
| `void Deserialize(const std::string& in) override` | 从字符串反序列化 |

---

## 六、事件机制（EventBus）

背包模块内部使用全局 `EventBus` 发布事件，外部可查询历史记录：

```cpp
#include "core/inventory/EventBus.h"

// 获取历史事件（测试/调试用）
for (const auto& e : EventBus::Instance().GetHistory()) {
    // 处理事件
}
EventBus::Instance().ClearHistory();
```

**事件类型枚举 `InventoryEventType`**：

| 值 | 触发时机 |
|---|---|
| `ItemAdded` | 物品添加成功 |
| `ItemRemoved` | 物品移除成功 |
| `ItemUsed` | 消耗品使用成功 |
| `ItemEquipped` | 装备穿戴成功 |
| `GoldChanged` | 金钱变动 |
| `InventoryFull` | 背包已满导致添加失败 |

**事件结构 `InventoryGameEvent`**：

| 字段 | 类型 | 说明 |
|---|---|---|
| `type` | `InventoryEventType` | 事件类型 |
| `itemId` | `std::string` | 物品ID |
| `count` | `int` | 涉及数量 |
| `characterId` | `std::string` | 目标角色ID（Use/Equip时） |
| `value` | `int` | 金钱变动量（GoldChanged时） |

---

## 七、UI 显示

推荐使用 `GameConsoleUI::ShowInventory` 显示背包信息：

```cpp
#include "core/ui/GameConsoleUI.h"

InventoryManager inv(20);
inv.AddItem("heal_herb", 5);
inv.AddItem("iron_sword", 1);
GameConsoleUI::ShowInventory(inv);
```

输出示例：
```
--- 当前背包 ---
容量: 2 / 20
金钱: 0
物品列表:
  [1] 止血草 (ID: heal_herb) x5 [消耗品]
  [2] 铁剑 (ID: iron_sword) x1 [装备]
----------------
```

---

## 八、完整使用示例

```cpp
#include "core/inventory/InventoryManager.h"
#include "core/inventory/ItemConfigLoader.h"
#include "core/inventory/EventBus.h"
#include "core/ui/GameConsoleUI.h"

// 1. 加载配置
ItemConfigLoader::LoadFromFile("assets/config/items.json");

// 2. 创建背包
InventoryManager inv(20);

// 3. 操作物品
inv.AddItem("heal_herb", 5);
inv.AddItem("iron_sword", 1);
inv.AddGold(100);

// 4. 查询
std::cout << "持有草药: " << inv.GetItemCount("heal_herb") << "\n";
std::cout << "金钱: " << inv.GetGold() << "\n";

// 5. 显示背包
GameConsoleUI::ShowInventory(inv);

// 6. 使用/装备（只发事件，效果由外部处理）
inv.UseItem("heal_herb", 1, "li_xiaoyao");
inv.EquipItem("iron_sword", "li_xiaoyao");

// 7. 存档
std::string save;
inv.Serialize(save);
```

---

## 九、注意事项

1. **"效果零执行"**：`UseItem` 和 `EquipItem` 只负责背包层面的扣除/计数和发事件，实际效果必须由外部监听者处理。
2. **容量按种类计算**：`IsFull()` 判断的是不同 `itemId` 的数量是否达到 `capacity`，同一物品堆叠不受此限制（受 `maxStack` 限制）。
3. **必须先加载配置**：所有物品操作前必须完成 `ItemConfigLoader::LoadFromFile`，否则 `AddItem` 会因找不到定义而失败。
4. **线程不安全**：`InventoryManager` 实例应在单线程中使用。
