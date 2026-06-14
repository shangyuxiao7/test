# PAL1 项目 — AI 开发指南

> **用途**：本文档供 AI 编码助手快速理解项目全貌、当前状态与开发任务。
> **更新日期**：2026-05-30
> **项目路径**：`E:\WORKKKKKKKKKK\Projects\pal_3ctrl\`

---

## 一、项目概述

PAL1 是一个回合制 RPG 游戏的核心框架项目，采用 **C++17/20** 开发，目标是将三个独立子模块（地图、背包、战斗）集成为一个可运行的总控制系统。

**核心原则**：
- 每个子模块有且仅有 **一个对外暴露的接口类**
- 模块内部零 I/O、零平台依赖，通过回调/事件与外部通信
- 总控制模块（`GameMain`）负责生命周期管理与事件路由，不涉具体业务逻辑

---

## 二、目录结构

```
pal_3ctrl/
├── pal_3ctrl.vcxproj              # VS 项目文件（已包含所有源文件）
├── pal_3ctrl.vcxproj.filters      # VS 筛选器（暂未按文件夹分层）
├── pal_3ctrl.slnx                 # VS 解决方案
│
├── code/
│   ├── main.cpp                   # 当前仅输出 HelloWorld，仅 include 三个接口头文件
│   │
│   ├── assets/
│   │   └── config/
│   │       ├── skills.json        # 战斗技能配置表（19 个技能）
│   │       ├── enemies.json       # 敌人与群组配置表（7 个敌人，7 个群组）
│   │       ├── items.json         # 物品配置表
│   │       └── maps.json          # 地图与房间配置表（2 张地图）
│   │
│   ├── foundation/                # 基础设施层（从地图模块提取）
│   │   ├── ISerializable.h        # 序列化接口基类
│   │   ├── JsonValue.h/.cpp       # 自研 JSON 解析器
│   │   ├── JsonUtils.h/.cpp
│   │   ├── SaveLoadSystem.h/.cpp
│   │   └── ConfigLoader.h/.cpp    # 地图模块的通用配置加载器
│   │
│   └── core/                      # 核心业务层
│       ├── battle/                # 战斗模块（21 个文件）
│       │   ├── BattleSession.h/.cpp       # 唯一对外接口
│       │   ├── GameController.h           # 核心状态机（ATB 行动条）
│       │   ├── BattleManager.h
│       │   ├── Character.h/.cpp
│       │   ├── PlayerCharacter.h
│       │   ├── AICharacter.h
│       │   ├── Action.h                   # 技能基类与派生
│       │   ├── Attributes.h
│       │   ├── StatusEffect.h
│       │   ├── GameEvent.h
│       │   ├── SkillLibrary.h
│       │   ├── EnemyFactory.h
│       │   ├── BattleDataTables.h/.cpp    # 原 DataTables，已重命名避免冲突
│       │   ├── BattleConfigLoader.h/.cpp  # 原 ConfigLoader，已重命名避免冲突
│       │   ├── JsonParser.h
│       │   ├── TurnManager.h
│       │   └── ...
│       │
│       ├── inventory/             # 背包模块（13 个文件）
│       │   ├── InventoryManager.h/.cpp    # 核心接口
│       │   ├── Item.h
│       │   ├── ItemDatabase.h/.cpp
│       │   ├── ItemConfigLoader.h/.cpp
│       │   ├── ItemDataTables.h/.cpp      # 原 DataTables，已重命名避免冲突
│       │   ├── EventBus.h/.cpp
│       │   ├── Types.h
│       │   └── ISerializable.h            # 副本（同级目录引用用）
│       │
│       └── world/                 # 地图模块（15 个文件）
│           ├── WorldFacade.h/.cpp         # 唯一对外接口（PIMPL）
│           ├── WorldManager.h/.cpp
│           ├── MapInstance.h/.cpp
│           ├── MapData.h/.cpp
│           ├── RoomGraph.h/.cpp
│           ├── Entity.h/.cpp
│           ├── WorldEvent.h
│           ├── WorldConfigLoader.h/.cpp
│           └── ...
```

---

## 三、各模块现状

### 3.1 战斗模块 (`core/battle/`)

| 维度 | 状态 |
|------|------|
| **核心功能** | 完整：ATB 行动条回合制、5 种技能类型、Buff/Debuff/DoT/HoT、JSON 配置驱动 |
| **对外接口** | `BattleSession`（创建-配置队伍/敌人-Start-每帧 Update-提交玩家输入） |
| **事件机制** | `DisplayCallback` 函数指针 + `GameEvent` 结构，零 I/O |
| **配置加载** | `BattleConfigLoader::LoadFromFile("assets/config/skills.json")` + `enemies.json` |
| **已知问题** | 无重大已知问题，编译已通过 |

**集成要点**：
- 进入战斗前调用配置加载器
- 通过 `SetDisplayCallback()` 绑定事件回调到 UI 层
- 玩家操作通过 `SubmitMainAction()` / `SubmitSkill()` / `SubmitTarget()` 提交
- 战斗结果通过 `GetResult()` 获取

---

### 3.2 背包模块 (`core/inventory/`)

| 维度 | 状态 |
|------|------|
| **核心功能** | 完整：格子制堆叠存储、添加/移除/使用/装备/丢弃、金钱系统、序列化存档 |
| **对外接口** | `InventoryManager`（构造时传入容量，所有操作返回 bool） |
| **事件机制** | `EventBus` 全局单例，发布 ItemAdded/Removed/Used/Equipped/GoldChanged/InventoryFull |
| **配置加载** | `ItemConfigLoader::LoadFromFile("assets/config/items.json")` |
| **已知问题** | 无重大已知问题，编译已通过 |

**集成要点**：
- "效果零执行"：背包只发事件，实际效果由外部监听者处理
- `ItemStack` 只存 ID 和数量，名称/类型等从 `ItemDataTables` 读取
- 使用 `Serialize()` / `Deserialize()` 支持存档读档

---

### 3.3 地图模块 (`core/world/`)

| 维度 | 状态 |
|------|------|
| **核心功能** | 完整：多地图/房间图、6 种实体类型、出入管道、全局标记、条件分支 |
| **对外接口** | `WorldFacade`（PIMPL 封装，LoadConfig-EnterMap-MoveToRoom-InteractWithEntity） |
| **事件机制** | `WorldEventCallback` + `WorldEvent`（对话/战斗/掉落/传送/通知请求） |
| **配置加载** | `WorldFacade::LoadConfig("assets/config/maps.json")` |
| **已知问题** | 无重大已知问题，编译已通过 |

**集成要点**：
- 管道（Pipeline）机制使地图可脚本化，策划改 JSON 即可实现剧情流程
- 实体交互后标记为 PROCESSED，支持全局持久化（`isGlobalState=true`）
- 出口可用性通过 `conditionFlag` 全局标记控制

---

## 四、已解决的重大集成问题

| 问题 | 解决方案 |
|------|---------|
| `DataTables` 类名冲突（战斗/背包各有一个） | 战斗重命名为 `BattleDataTables`，背包重命名为 `ItemDataTables` |
| `ConfigLoader.cpp` 同名冲突 | 战斗的 ConfigLoader 重命名为 `BattleConfigLoader` |
| 地图模块引用 `../foundation/` 路径失效 | 全部改为 `../../foundation/`（因 world/ 现在在 core/ 下） |
| `ItemDataTables.cpp` 被过度替换为 `ItemItemDataTables` | 已重写整个文件 |
| `ItemDatabase.h` 仍引用旧的 `
