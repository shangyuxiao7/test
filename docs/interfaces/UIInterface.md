# UI 模块接口文档 (UI Module)

> **对外接口类**: `GameConsoleUI`（通用控制台 UI）、`BattleConsoleUI`（战斗控制台 UI）  
> **模块路径**: `core/ui/`  
> **职责边界**: 项目中**唯一允许直接进行控制台 I/O** 的模块。负责将各业务模块的数据结构渲染为可读的文本界面，并收集用户键盘输入。

---

## 一、模块概述

UI 模块是业务模块与玩家之间的桥梁。设计原则：

- **业务模块零 I/O**：`core/battle/`、`core/character/`、`core/inventory/`、`core/world/` 内部绝不直接 `std::cout`/`std::cin`
- **统一渲染入口**：所有显示逻辑收拢到 `GameConsoleUI` 和 `BattleConsoleUI`
- **输入即时提交**：`BattleConsoleUI` 在渲染玩家菜单时会直接读取输入并提交给 `BattleSession` 或 `GameManager`

---

## 二、依赖关系树状图

```
                              main / GameManager
                                     │
                    ┌────────────────┼────────────────┐
                    │                │                │
            GameConsoleUI    BattleConsoleUI    业务模块回调
                    │                │                │
         ┌──────────┼──────────┐    │         ┌──────┴──────┐
         │          │          │    │         │             │
      队伍显示   背包显示    地图显示  │      战斗事件      世界事件
         │          │          │    │         │             │
    PartyManager InventoryManager WorldFacade │      WorldFacade
         │          │          │    │                         │
         └──────────┴──────────┘    │                    BattleSession
                    │               │                         │
              Character           GameManager              GameController
```

---

## 三、通用 UI：`GameConsoleUI`

纯静态类，所有方法均为 `static`，无需实例化。

### 3.1 辅助方法

| 方法 | 说明 |
|---|---|
| `static void PrintSeparator()` | 打印一行 `========================================` 分隔符 |

### 3.2 主菜单

| 方法 | 说明 |
|---|---|
| `static void ShowMainMenu()` | 打印主菜单（开始新游戏 / 运行自动测试 / 退出），并在末尾输出 `"请选择: "` |

### 3.3 队伍显示

| 方法 | 参数 | 说明 |
|---|---|---|
| `static void ShowParty(PartyManager& party)` | 队伍管理器引用 | 打印当前所有成员的详细信息（ID、等级、HP/QI、攻防速、参战/待命/死亡状态） |

### 3.4 背包显示

| 方法 | 参数 | 说明 |
|---|---|---|
| `static void ShowInventory(const InventoryManager& inv)` | 背包管理器引用 | 打印背包容量、金钱、物品列表（含类型中文名） |

### 3.5 地图房间显示

| 方法 | 参数 | 说明 |
|---|---|---|
| `static void ShowWorldRoom(const WorldFacade& world)` | 世界 facade 引用 | 打印当前地图名、房间名、房间描述、可用出口列表、可交互实体列表 |

### 3.6 世界事件显示

| 方法 | 参数 | 说明 |
|---|---|---|
| `static void ShowWorldEvent(const WorldEvent& e)` | 世界事件 | 根据事件类型打印 `[剧情]` / `[战斗]` / `[掉落]` / `[传送]` / `[通知]` 前缀的文本 |

### 3.7 子菜单

| 方法 | 说明 |
|---|---|
| `static void ShowPartyMenu()` | 打印队伍管理菜单（查看/切换/装备/卸下/返回），并在末尾输出 `"请选择: "` |
| `static void ShowInventoryMenu()` | 打印背包菜单（查看/使用/丢弃/返回），并在末尾输出 `"请选择: "` |
| `static void ShowGameMenu()` | 打印游戏内主菜单（移动/交互/队伍/背包/返回），并在末尾输出 `"请选择: "` |

---

## 四、战斗 UI：`BattleConsoleUI`

纯静态类，负责战斗画面的完整渲染和玩家输入收集。

### 4.1 准备工作

在使用 `RunManualBattle` 前，通常不需要手动调用 `Bind`，因为 `RunManualBattle` 内部会自动绑定。

| 方法 | 参数 | 说明 |
|---|---|---|
| `static void Bind(BattleSession& session)` | 战斗会话引用 | 将 `session.SetDisplayCallback` 绑定到 `BattleConsoleUI` 内部的事件处理器，用于收集战斗日志 |
| `static void ClearLogs()` | - | 清空内部战斗日志缓存 |

### 4.2 运行手动战斗（推荐）

| 方法 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `static BattleOutcome RunManualBattle(BattleSession& session)` | 战斗会话引用 | `BattleOutcome::Victory` / `Defeat` / `Fled` | 自动渲染战斗画面、收集玩家输入、驱动战斗到结束 |
| `static BattleOutcome RunManualBattle(GameManager& gm)` | 游戏管理器引用 | 同上 | 同上，但战斗结束后自动调用 `gm.FinalizeBattle()` 进行战后结算 |

**`BattleOutcome` 枚举**：

| 值 | 含义 |
|---|---|
| `Victory` | 战斗胜利 |
| `Defeat` | 战斗失败（全灭） |
| `Fled` | 玩家逃跑 |

### 4.3 运行自动战斗

| 方法 | 参数 | 说明 |
|---|---|---|
| `static void RunAutoBattle(BattleSession& session)` | 战斗会话引用 | AI 控制玩家全部选择普通攻击，直到战斗结束。用于测试或挂机 |

### 4.4 渲染行为说明

`RunManualBattle` 每帧会自动执行以下操作：

1. **清屏**（Windows 下 `cls`，Linux 下 `clear`）
2. **绘制行动顺序**：显示接下来 5 个行动的角色名和预计时间
3. **绘制双方队伍面板**：
   - 敌方：名称、HP、攻击、防御、Buff 状态
   - 我方：名称、HP、气力、攻击、防御、Buff 状态
4. **绘制战斗日志**：显示最近 20 条战斗事件
5. **根据当前阶段绘制交互菜单**：
   - `PlayerMainMenu`：显示 1.普通攻击 2.技能 3.回气 0.自动运行 -1.逃跑，读取输入并提交
   - `PlayerSkillMenu`：显示技能列表（含气力消耗、目标类型、可用状态），读取输入并提交
   - `PlayerTargetSelect`：显示候选目标列表（含 HP、攻防、Buff），读取输入并提交

---

## 五、完整使用示例

### 5.1 通用 UI 示例

```cpp
#include "core/ui/GameConsoleUI.h"
#include "core/character/PartyManager.h"
#include "core/inventory/InventoryManager.h"
#include "core/world/WorldFacade.h"

PartyManager party;
InventoryManager inv(20);
WorldFacade world;

// 主菜单
GameConsoleUI::ShowMainMenu();
int choice; std::cin >> choice;

// 显示队伍
GameConsoleUI::ShowParty(party);

// 显示背包
GameConsoleUI::ShowInventory(inv);

// 显示当前地图房间
world.EnterMap("demo_village", "village_start");
GameConsoleUI::ShowWorldRoom(world);

// 显示事件
WorldEvent e{WorldEventType::RequestDialogue, "你好，旅行者！", 0, ""};
GameConsoleUI::ShowWorldEvent(e);   // 输出: [剧情] 你好，旅行者！
```

### 5.2 战斗 UI 示例（独立战斗测试）

```cpp
#include "core/ui/BattleConsoleUI.h"
#include "core/battle/BattleSession.h"
#include "core/battle/BattleConfigLoader.h"

// 1. 加载配置
SkillConfigLoader::LoadFromFile("assets/config/skills.json");
EnemyConfigLoader::LoadFromFile("assets/config/enemies.json");

// 2. 创建战斗会话
BattleSession session;
int p1 = session.AddPlayer("李逍遥", 120, 60, 12, 25, 8);
session.AddPlayerSkill(p1, "slash");
session.AddPlayerSkill(p1, "fireball");
session.AddEnemyGroup("novice");

// 3. 运行手动战斗（UI 会自动渲染画面、收集输入、驱动战斗）
BattleOutcome outcome = BattleConsoleUI::RunManualBattle(session);
if (outcome == BattleOutcome::Victory) {
    std::cout << "战斗胜利！\n";
} else if (outcome == BattleOutcome::Defeat) {
    std::cout << "战斗失败...\n";
} else {
    std::cout << "逃跑了\n";
}
```

### 5.3 战斗 UI 示例（通过 GameManager）

```cpp
#include "core/ui/BattleConsoleUI.h"
#include "core/game/GameManager.h"

GameManager gm;
gm.Initialize(&party, &inventory, &world);

// 世界事件触发战斗后...
if (gm.IsInBattle()) {
    BattleOutcome outcome = BattleConsoleUI::RunManualBattle(gm);
    // 战后结算（FinalizeBattle）已在 RunManualBattle 内部自动调用
}
```

### 5.4 自动战斗示例

```cpp
// 快速测试：让 AI 自动打完一场战斗
BattleSession session;
session.AddPlayer("李逍遥", 200, 80, 15, 30, 10);
session.AddEnemyGroup("boss");
BattleConsoleUI::RunAutoBattle(session);
auto result = session.GetResult();
std::cout << (result.playerWon ? "胜利" : "失败") << "\n";
```

---

## 六、注意事项

1. **线程不安全**：`GameConsoleUI` 和 `BattleConsoleUI` 都是静态类，内部含静态状态（如 `eventLogs`），应在单线程中使用。
2. **BattleConsoleUI 会清屏**：`RunManualBattle` 每次渲染都会调用系统 `cls`/`clear` 命令，如果外部有其他输出，可能会被清掉。
3. **输入阻塞**：`RunManualBattle` 在玩家回合时会阻塞等待 `std::cin`，直到玩家输入有效数字。
4. **`ShowParty` 参数为非 const**：因为 `PartyManager::GetMember` 返回非 const 指针，所以 `ShowParty` 接收 `PartyManager&`。如果只有 const 引用，需在外部自行实现显示逻辑。
5. **日志缓存上限**：`BattleConsoleUI` 内部只缓存最近 20 条战斗日志，更早的日志会被丢弃。
