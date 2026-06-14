# 回合制战斗游戏项目结构说明（重构版）

## 1. 项目概述
本项目是一个基于控制台的回合制战斗游戏，采用面向对象设计，将计算逻辑与显示逻辑分离。玩家控制友方角色，敌方由AI控制，战斗按速度排序决定行动顺序。核心流程由 `GameController` 驱动，`BattleManager` 管理队伍与胜负，`TurnManager` 控制行动顺序，`Character` 及其派生类封装战斗计算，`Action` 实现具体行为，所有输入输出集中在 `BattleUI` 中，并通过事件回调 (`GameEvent` + `DisplayCallback`) 与计算层解耦。

**本次重构**：引入 `ICombatant` 接口，彻底消除 `Character` 与 `Action` 之间的循环依赖，使 `Action` 仅依赖抽象接口，提高了可扩展性和可测试性。

## 2. 类/结构体职责清单

| 类/结构体 | 所在文件 | 职责 |
|-----------|----------|------|
| `Attributes` | `Attributes.h` | 纯数据容器，存储角色的生命、气力、攻击、防御、速度等属性，不包含任何逻辑。 |
| `Faction` | `GameEvent.h` | 阵营枚举（玩家、敌方）。 |
| `EventType` | `GameEvent.h` | 事件类型枚举（伤害、治疗、死亡、回合开始等）。 |
| `GameEvent` | `GameEvent.h` | 事件数据结构，携带角色名、目标名、数值变化、当前状态等信息，用于从计算层传递到显示层。 |
| `DisplayCallback` | `GameEvent.h` | 显示回调函数类型，`std::function<void(const GameEvent&)>`。 |
| `ICombatant` | `ICombatant.h` | **战斗单位抽象接口**，定义所有战斗单位必须提供的方法（属性查询、受到伤害、攻击等）。不依赖任何其他自定义类，是最底层接口。 |
| `Action` | `Action.h` | 行为抽象基类。包含名称、气力消耗和纯虚 `Execute(ICombatant*, ICombatant*)` 方法。依赖 `ICombatant`，不再依赖具体 `Character`。 |
| `NormalAttack` | `Action.h` | `Action` 的派生类，实现普通攻击（消耗0气力），调用使用者的 `Attack` 方法。 |
| `Character` | `Character.h` | 角色抽象基类，实现 `ICombatant` 接口。封装属性、状态、伤害/治疗/气力计算、事件通知；提供纯虚接口 `TakeTurn`、`SelectTarget`、`SelectAction`。依赖 `Attributes`、`GameEvent` 和 `Action`（拥有动作列表）。 |
| `PlayerCharacter` | `PlayerCharacter.h` | 玩家控制角色，继承 `Character`。`SelectTarget` 返回第一个存活敌人，`SelectAction` 返回第一个可用动作（实际选择由 UI 交互决定）。 |
| `AICharacter` | `AICharacter.h` | AI 控制角色，继承 `Character`。`SelectTarget` 随机选择存活敌人，`SelectAction` 返回第一个可用动作。 |
| `TurnManager` | `TurnManager.h` | 行动顺序管理器。按速度降序初始化队列，提供 `GetNextActor` 按序返回存活角色，支持重置回合。依赖 `Character`。 |
| `BattleManager` | `BattleManager.h` | 战斗管理器（旧版）。管理队伍、初始化 `TurnManager`、执行战斗循环、检查胜负。**注意：在最终实现中，实际流程控制由 `GameController` 承担，本类未被完全使用。** |
| `GameController` | `GameController.h` | **战斗流程总控制器**。整合 `BattleManager` 和 `TurnManager`，驱动回合循环，处理玩家输入（通过 `BattleUI`），执行敌方 AI 行动，判定胜负并清理资源。 |
| `BattleUI` | `BattleUI.h` | 所有控制台输入输出集中于此。包含静态方法：显示战场状态（队伍面板+行动顺序）、选择行动、选择目标、显示战斗开始/结束、暂停等。依赖 `Character`、`Action`、`TurnManager` 等。 |
| `GameDisplay` | `BattleUI.h` | 静态类，将 `GameEvent` 转换为用户可见文本，通过 `BattleUI` 的对应方法输出。作为 `DisplayCallback` 的实现。 |
| `main` | `pal_20260405.cpp` | 程序入口。创建角色、设置回调、将角色加入 `GameController`、启动战斗、清理内存。 |

## 3. 依赖关系树状图（消除循环依赖后）

                                    main
                                      │
                              GameController
                    ┌───────────┼───────────┐
                    │           │           │
                BattleUI    BattleManager  TurnManager
                    │           │           │
                    └─────┬─────┘           │
                          │                 │
                    ┌─────┼─────┐           │
                    │     │     │           │
              PlayerCharacter AICharacter   │
                    │     │     │           │
                    └─────┼─────┘           │
                          │                 │
                      Character ◄───────────┘
                          │
                    ┌─────┴─────┐
                    │           │
                Attributes  GameEvent
                    │           │
                    └─────┬─────┘
                          │
                    ┌─────┴─────┐
                    │           │
                ICombatant   Action
                    ▲           │
                    └───────────┘
                          │
                    NormalAttack

**说明**：
- **箭头方向**表示依赖（上层依赖下层），子类（`PlayerCharacter`、`AICharacter`）位于父类（`Character`）上方，体现继承关系。
- **最底层**：`ICombatant`、`Attributes`、`GameEvent` 不依赖任何项目内其他类。
- **无循环依赖**：`Action` 只依赖 `ICombatant`，`Character` 实现 `ICombatant` 并依赖 `Action`，依赖方向始终向下。
- **继承关系**：`NormalAttack` 继承自 `Action`，`PlayerCharacter` 和 `AICharacter` 继承自 `Character`。

## 4. 核心流程说明

1. **初始化**：`main` 中通过 `CreatePlayer`/`CreateEnemy` 创建角色，每个角色添加 `NormalAttack` 动作，设置显示回调为 `GameDisplay::Show`，然后调用 `GameController::AddCharacter` 加入控制器。
2. **开始战斗**：`GameController::StartBattle` 收集所有角色，初始化 `TurnManager` 按速度排序，调用 `BattleUI::ShowBattleStart` 显示初始状态。
3. **回合循环**：
   - 调用 `TurnManager::GetNextActor` 获取下一个存活角色。
   - 如果是玩家角色：调用 `ProcessPlayerTurn`，通过 `BattleUI` 选择行动（当前只有普通攻击）和目标，执行 `Action::Execute`。
   - 如果是敌方角色：调用 `ProcessEnemyTurn`，自动选择目标并调用 `Character::PerformAttack`（内部调用 `Attack(ICombatant*)`）。
   - 每次攻击触发 `Character::Notify` 发送 `GameEvent`，由 `GameDisplay::Show` 转换为 UI 输出。
4. **胜负判定**：每次行动后调用 `CheckAndHandleVictory`，若一方全灭则显示结果并结束循环。
5. **清理**：`GameController::Cleanup` 删除所有动态分配的 `Character` 和 `Action` 对象。

## 5. 设计特点

- **计算与显示完全分离**：`Character` 及派生类只处理数值和状态变更，不包含任何 I/O 操作。
- **事件驱动显示**：通过 `DisplayCallback` 回调传递 `GameEvent`，便于扩展为图形界面。
- **接口隔离**：引入 `ICombatant` 接口，使 `Action` 不依赖具体 `Character`，消除了循环依赖，增强了可扩展性（可对任何实现 `ICombatant` 的对象执行动作）。
- **策略模式**：`SelectTarget` 和 `SelectAction` 在 `PlayerCharacter` 和 `AICharacter` 中提供不同实现，决定行为差异。
- **单一职责**：每个类职责明确，`TurnManager` 只负责顺序，`BattleUI` 只负责显示/交互，`GameController` 负责流程编排。
- **可扩展性**：新增技能只需派生 `Action`，新增角色类型只需派生 `Character` 或实现 `ICombatant`，无需修改核心流程。

## 6. 文件清单

| 文件 | 包含的主要类/结构体 |
|------|---------------------|
| `Attributes.h` | `Attributes` |
| `GameEvent.h` | `Faction`, `EventType`, `GameEvent`, `DisplayCallback` |
| `ICombatant.h` | `ICombatant` |
| `Character.h` | `Character` |
| `Character.cpp` | `Character::AddAction` 实现 |
| `Action.h` | `Action`, `NormalAttack` |
| `PlayerCharacter.h` | `PlayerCharacter` |
| `AICharacter.h` | `AICharacter` |
| `TurnManager.h` | `TurnManager` |
| `BattleManager.h` | `BattleManager` |
| `BattleUI.h` | `BattleUI`, `GameDisplay` |
| `GameController.h` | `GameController` |
| `pal_20260405.cpp` | `main` 及辅助创建函数 |