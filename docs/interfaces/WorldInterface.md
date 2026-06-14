# 地图模块接口文档 (World Module)

> **对外接口类**: `WorldFacade`（唯一暴露类，PIMPL 封装）  
> **配套 UI**: `GameConsoleUI::ShowWorldRoom` / `ShowWorldEvent`（`core/ui/GameConsoleUI.h`）  
> **模块路径**: `core/world/`  
> **配置路径**: `assets/config/maps.json`

---

## 一、模块概述

多地图/房间图探索系统。支持：

- 多张地图，每张地图由多个房间（Room）组成
- 6 种实体类型（NPC、宝箱、出口、剧情点等）
- 条件分支出口（通过全局标记控制可用性）
- 管道（Pipeline）机制：实体交互后标记为 `PROCESSED`，支持全局持久化
- 纯事件驱动：所有交互结果通过 `WorldEventCallback` 回调通知外部

**零 I/O 原则**：`core/world/` 内部不直接输出，通过 `WorldEventCallback` 抛事件给外部。推荐使用 `core/ui/GameConsoleUI` 作为配套渲染器。

---

## 二、依赖关系树状图

```
                              main / GameManager
                                     │
                    ┌────────────────┼────────────────┐
                    │                │                │
              GameConsoleUI    WorldFacade（唯一对外接口，PIMPL）  GameManager
                    │                │                │
           ShowWorldRoom       WorldManager      HandleWorldEvent
           ShowWorldEvent            │                │
                    │          ┌─────┴─────┐         │
                    │          │           │         │
                    │      MapInstance   WorldEvent   │
                    │          │                        │
                    │    ┌─────┴─────┐                  │
                    │    │           │                  │
                    │ RoomGraph     Entity              │
                    │    │                              │
                    │ MapData                            │
                    │    │                              │
                    │ WorldConfigLoader                  │
                    │    │                              │
                    │ assets/config/maps.json            │
```

---

## 三、配置加载

```cpp
#include "core/world/WorldFacade.h"

WorldFacade world;
bool ok = world.LoadConfig("assets/config/maps.json");
```

| 方法 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `bool LoadConfig(const std::string& filepath)` | JSON 文件路径 | bool | 从文件加载地图配置 |
| `bool LoadConfigFromString(const std::string& jsonContent)` | JSON 字符串 | bool | 从字符串加载（测试用） |

---

## 四、核心接口类：`WorldFacade`

### 4.1 构造与生命周期

```cpp
WorldFacade world;
```

| 方法 | 说明 |
|---|---|
| `WorldFacade()` | 默认构造 |
| `~WorldFacade()` | 析构 |
| `WorldFacade(const WorldFacade&) = delete` | 禁止拷贝 |
| `WorldFacade& operator=(const WorldFacade&) = delete` | 禁止赋值 |

### 4.2 地图进出

```cpp
world.EnterMap("demo_village", "village_start");   // 进入地图，从指定房间开始
world.LeaveMap();                                   // 离开当前地图
```

| 方法 | 参数 | 说明 |
|---|---|---|
| `void EnterMap(const std::string& mapId, const std::string& entryRoomId)` | 地图ID、入口房间ID | 进入指定地图的指定房间 |
| `void LeaveMap()` | - | 离开当前地图 |
| `bool IsInMap() const` | - | 当前是否在地图中 |
| `std::string GetCurrentMapName() const` | - | 获取当前地图名称 |

### 4.3 移动与交互

```cpp
bool moved = world.MoveToRoom(0);           // 通过第0个出口移动
bool interacted = world.InteractWithEntity(1); // 与第1个实体交互
```

| 方法 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `bool MoveToRoom(int exitIndex)` | 出口索引（从0开始） | bool | 移动到目标房间。失败原因：索引越界、出口条件不满足 |
| `bool InteractWithEntity(int entityIndex)` | 实体索引（从0开始） | bool | 与实体交互。可能触发 `WorldEvent`，通过回调或 `GetPendingEvent` 获取 |

### 4.4 事件处理

```cpp
// 方式一：设置回调（推荐）
world.SetEventCallback([](const WorldEvent& e) {
    switch (e.type) {
        case WorldEventType::RequestDialogue:
            std::cout << "[剧情] " << e.param1 << "\n";
            break;
        case WorldEventType::RequestBattle:
            std::cout << "[战斗] " << e.param1 << " (波数:" << e.param2 << ")\n";
            break;
        case WorldEventType::RequestLoot:
            std::cout << "[掉落] " << e.param1 << " x" << e.param2 << "\n";
            break;
        case WorldEventType::RequestTransferMap:
            std::cout << "[传送] " << e.param1 << " -> " << e.param3 << "\n";
            break;
        case WorldEventType::Notification:
            std::cout << "[通知] " << e.param1 << "\n";
            break;
    }
});

// 推荐做法：使用 GameConsoleUI 显示事件
world.SetEventCallback([](const WorldEvent& e) {
    GameConsoleUI::ShowWorldEvent(e);
});

// 方式二：轮询检查
if (world.HasPendingEvent()) {
    WorldEvent e = world.GetPendingEvent();
    world.NotifyEventComplete(true);   // 告诉模块事件已被处理
}
```

| 方法 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `void SetEventCallback(WorldEventCallback cb)` | 回调函数 | - | 绑定事件回调 |
| `bool HasPendingEvent() const` | - | bool | 是否有未处理的事件 |
| `WorldEvent GetPendingEvent() const` | - | `WorldEvent` | 获取待处理事件 |
| `void NotifyEventComplete(bool success)` | 是否成功处理 | - | 通知模块事件已完成（部分实体会据此标记为 PROCESSED） |

**回调签名**：

```cpp
using WorldEventCallback = std::function<void(const WorldEvent&)>;
```

**事件结构 `WorldEvent`**：

| 字段 | 类型 | 说明 |
|---|---|---|
| `type` | `WorldEventType` | 事件类型 |
| `param1` | `std::string` | 主参数（对话内容/战斗群组ID/掉落物品ID/目标地图ID/通知文本） |
| `param2` | `int` | 数值参数（掉落数量/战斗波数等） |
| `param3` | `std::string` | 附加参数（目标房间ID等） |

**事件类型枚举 `WorldEventType`**：

| 值 | 含义 | param1 含义 | param2 含义 | param3 含义 |
|---|---|---|---|---|
| `None` | 无事件 | - | - | - |
| `RequestDialogue` | 请求播放剧情对话 | 对话文本 | - | - |
| `RequestBattle` | 请求进入战斗 | 敌群组ID | 波数 | - |
| `RequestLoot` | 请求发放掉落 | 物品ID | 数量 | - |
| `RequestTransferMap` | 请求传送地图 | 目标地图ID | - | 目标房间ID |
| `Notification` | 普通通知 | 通知文本 | - | - |

### 4.5 状态查询（用于UI展示）

```cpp
auto room = world.GetCurrentRoom();
auto exits = world.GetAvailableExits();
auto entities = world.GetRoomEntities();
```

| 方法 | 返回值 | 说明 |
|---|---|---|
| `RoomDisplayInfo GetCurrentRoom() const` | 当前房间信息 | 名称与描述 |
| `std::vector<ExitDisplayInfo> GetAvailableExits() const` | 出口列表 | 仅包含当前可用的出口 |
| `std::vector<EntityDisplayInfo> GetRoomEntities() const` | 实体列表 | 当前房间的所有实体 |

**`RoomDisplayInfo` 结构**：

| 字段 | 类型 | 说明 |
|---|---|---|
| `name` | `std::string` | 房间名称 |
| `description` | `std::string` | 房间描述 |

**`ExitDisplayInfo` 结构**：

| 字段 | 类型 | 说明 |
|---|---|---|
| `description` | `std::string` | 出口描述 |
| `targetRoomName` | `std::string` | 目标房间名称 |
| `isAvailable` | `bool` | 是否可用（受条件标记控制） |

**`EntityDisplayInfo` 结构**：

| 字段 | 类型 | 说明 |
|---|---|---|
| `id` | `std::string` | 实体唯一ID |
| `name` | `std::string` | 显示名称 |
| `description` | `std::string` | 描述 |
| `typeName` | `std::string` | 类型名称 |
| `isInteractable` | `bool` | 是否可交互 |

### 4.6 全局标记系统

用于控制条件出口和剧情分支：

```cpp
world.SetGlobalFlag("has_key", 1);
int flag = world.GetGlobalFlag("has_key");   // 返回 1
```

| 方法 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `void SetGlobalFlag(const std::string& flag, int value)` | 标记名、值 | - | 设置全局标记 |
| `int GetGlobalFlag(const std::string& flag) const` | 标记名 | int | 获取全局标记（不存在返回0） |
| `std::vector<std::pair<std::string, int>> GetAllGlobalFlags() const` | - | 全部标记 | 调试用 |

### 4.7 实体处理标记

```cpp
world.MarkEntityProcessed("chest_001");          // 手动标记实体已处理
bool done = world.IsEntityProcessed("chest_001"); // 查询是否已处理
```

| 方法 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `void MarkEntityProcessed(const std::string& entityId)` | 实体ID | - | 手动标记实体为已处理 |
| `bool IsEntityProcessed(const std::string& entityId) const` | 实体ID | bool | 查询实体是否已处理 |

### 4.8 调试辅助

| 方法 | 返回值 | 说明 |
|---|---|---|
| `std::string GetDebugWorldState() const` | JSON 格式字符串 | 输出完整内部状态（调试用） |

### 4.9 存档接口

```cpp
std::string saveData;
world.Serialize(saveData);     // 导出为字符串

WorldFacade world2;
world2.Deserialize(saveData);  // 从字符串恢复（包括全局标记、实体处理状态）
```

| 方法 | 说明 |
|---|---|
| `void Serialize(std::string& out) const` | 序列化为字符串 |
| `void Deserialize(const std::string& in)` | 从字符串反序列化 |

---

## 五、UI 显示

推荐使用 `GameConsoleUI` 显示地图房间信息和事件：

```cpp
#include "core/ui/GameConsoleUI.h"

WorldFacade world;
world.LoadConfig("assets/config/maps.json");
world.EnterMap("demo_village", "village_start");

// 显示当前房间
GameConsoleUI::ShowWorldRoom(world);

// 显示事件
WorldEvent e{WorldEventType::RequestDialogue, "你好，旅行者！", 0, ""};
GameConsoleUI::ShowWorldEvent(e);   // 输出: [剧情] 你好，旅行者！
```

`ShowWorldRoom` 输出示例：
```
当前: [demo_village] 村庄入口
这里是李逍遥的家乡，一个宁静的小村庄。

出口:
  1. 通往村口 -> village_gate

可交互:
  1. [NPC] 老村长
```

---

## 六、完整使用示例

```cpp
#include "core/world/WorldFacade.h"
#include "core/ui/GameConsoleUI.h"

// 1. 创建并加载配置
WorldFacade world;
if (!world.LoadConfig("assets/config/maps.json")) {
    std::cerr << "地图配置加载失败\n";
    return;
}

// 2. 绑定事件回调（使用 UI 模块显示）
world.SetEventCallback([](const WorldEvent& e) {
    GameConsoleUI::ShowWorldEvent(e);
});

// 3. 进入地图
world.EnterMap("demo_village", "village_start");

// 4. 展示当前房间
GameConsoleUI::ShowWorldRoom(world);

// 5. 查看出口并移动
auto exits = world.GetAvailableExits();
for (size_t i = 0; i < exits.size(); ++i) {
    std::cout << i << ": " << exits[i].description
              << " -> " << exits[i].targetRoomName
              << (exits[i].isAvailable ? "" : " [锁定]") << "\n";
}
world.MoveToRoom(0);

// 6. 与实体交互
auto entities = world.GetRoomEntities();
for (size_t i = 0; i < entities.size(); ++i) {
    std::cout << i << ": [" << entities[i].typeName << "] "
              << entities[i].name << "\n";
}
if (!entities.empty() && entities[0].isInteractable) {
    world.InteractWithEntity(0);
}

// 7. 设置全局标记解锁出口
world.SetGlobalFlag("has_key", 1);

// 8. 存档
std::string save;
world.Serialize(save);
```

---

## 七、注意事项

1. **PIMPL 封装**：`WorldFacade` 内部实现完全隐藏，只能通过公开接口操作。
2. **事件必须通知完成**：通过 `InteractWithEntity` 触发的事件，外部处理完毕后应调用 `NotifyEventComplete(true)`，否则部分实体可能不会标记为已处理。
3. **全局标记跨地图持久**：`SetGlobalFlag` 设置的标记会随 `Serialize`/`Deserialize` 保存，可用于跨地图剧情锁。
4. **实体处理状态持久**：标记为 `PROCESSED` 的实体（尤其是 `isGlobalState=true` 的）在存档读档后会保持已处理状态。
5. **条件出口自动判断**：出口可用性由配置中的 `conditionFlag` 和当前全局标记值自动计算，外部无需手动判断。
