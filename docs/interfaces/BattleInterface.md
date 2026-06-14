# 战斗模块接口文档 (Battle Module)

> **对外接口类**: `BattleSession`（业务逻辑唯一暴露类）、`BattleConsoleUI`（UI 渲染器）  
> **模块路径**: `core/battle/`、`core/ui/`  
> **配置路径**: `assets/config/skills.json`, `assets/config/enemies.json`

---

## 一、模块概述

ATB（Active Time Battle）行动条回合制战斗系统。外部只需与 `BattleSession` 交互，所有战斗逻辑（行动条推进、AI决策、技能结算、Buff/Debuff/DoT/HoT）均在模块内部完成。

**零 I/O 原则**：`core/battle/` 内部不直接输出，通过 `DisplayCallback` 回调把战斗事件抛给外部。推荐使用 `core/ui/BattleConsoleUI.h` 作为配套 UI 渲染器。

---

## 二、依赖关系树状图

```
                              main / GameManager
                                     │
                    ┌────────────────┼────────────────┐
                    │                │                │
            BattleConsoleUI    BattleSession    业务回调
                    │                │                │
                    └────────────────┘                │
                                     │                │
                              GameController          │
                   ┌───────────┼───────────┐        │
                   │           │           │        │
            BattleManager   ActionGaugeSystem  Character
                   │                           │
            TurnManager（保留但不活跃）         │
                   │                    ┌─────┴─────┐
                   │                    │           │
                   │              PlayerCharacter  AICharacter
                   │                    │
                   └────────────────────┘
                                       │
                         ┌─────────────┼─────────────┐
                         │             │             │
                    Attributes    GameEvent      StatusEffect
                         │             │             │
                         └─────────────┴─────────────┘
                                       │
                                 ICombatant
                                       │
                                 Action 派生类
                                       │
              ┌──────────┬─────────────┼─────────────┬──────────┐
              │          │             │             │          │
        NormalAttack  DamageSkill   HealSkill    BuffSkill  CompositeSkill
              │          │             │             │          │
              └──────────┴─────────────┴─────────────┴──────────┘
                                       │
                                 SkillLibrary
                                       │
                            BattleConfigLoader
                                       │
                            BattleDataTables
```

---

## 三、配置加载

战斗依赖两组配置，需在创建 `BattleSession` 前加载：

```cpp
#include "core/battle/BattleConfigLoader.h"

// 加载技能表和敌人/群组配置
SkillConfigLoader::LoadFromFile("assets/config/skills.json");
EnemyConfigLoader::LoadFromFile("assets/config/enemies.json");
```

| 类 | 方法 | 说明 |
|---|---|---|
| `SkillConfigLoader` | `static bool LoadFromFile(const std::string& filepath)` | 加载技能配置 |
| `SkillConfigLoader` | `static bool LoadFromString(const std::string& content)` | 从字符串加载（测试用） |
| `EnemyConfigLoader` | `static bool LoadFromFile(const std::string& filepath)` | 加载敌人/群组配置 |
| `EnemyConfigLoader` | `static bool LoadFromString(const std::string& content)` | 从字符串加载（测试用） |

---

## 四、核心接口类：`BattleSession`

### 4.1 构造与生命周期

```cpp
#include "core/battle/BattleSession.h"

BattleSession session;
```

| 方法 | 说明 |
|---|---|
| `BattleSession()` | 默认构造 |
| `~BattleSession()` | 析构，自动清理内部资源 |
| `BattleSession(const BattleSession&) = delete` | 禁止拷贝 |
| `BattleSession& operator=(const BattleSession&) = delete` | 禁止赋值 |

### 4.2 队伍配置（战斗开始前调用）

```cpp
// 添加一名我方角色（参数：名称、HP、气力、速度、攻击、防御）
int playerIndex = session.AddPlayer("李逍遥", 120, 60, 12, 25, 8);

// 给该角色添加技能（skillId 需存在于 skills.json）
session.AddPlayerSkill(playerIndex, "slash");
session.AddPlayerSkill(playerIndex, "fireball");

// 添加一组敌人（groupId 需存在于 enemies.json）
session.AddEnemyGroup("novice");
```

| 方法 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `int AddPlayer(const std::string& name, int hp, int qi, int speed, int attack, int defense)` | 角色名与基础六维 | 玩家索引（从0开始） | 添加我方战斗成员 |
| `void AddPlayerSkill(int playerIndex, const std::string& skillId)` | 玩家索引、技能ID | - | 给指定玩家追加可用技能 |
| `void AddEnemyGroup(const std::string& groupId)` | 敌群组ID | - | 按配置生成一组敌人 |

### 4.3 事件回调

模块内部不直接输出，通过 `DisplayCallback` 把战斗事件抛给外部。

**回调签名**：

```cpp
using DisplayCallback = std::function<void(const BattleGameEvent&)>;
```

**设置方式**：

```cpp
session.SetDisplayCallback([](const BattleGameEvent& e) {
    // 处理战斗事件：伤害、治疗、死亡、回合开始等
    std::cout << e.message << "\n";
});
```

**推荐做法**：使用 `BattleConsoleUI` 接管回调，无需手动设置：

```cpp
BattleConsoleUI::Bind(session);   // 内部自动设置 DisplayCallback
```

**事件结构 `BattleGameEvent`**：

| 字段 | 类型 | 说明 |
|---|---|---|
| `type` | `BattleEventType` | 事件类型（见下方枚举） |
| `characterName` | `std::string` | 触发者名称 |
| `targetName` | `std::string` | 目标名称 |
| `value` | `int` | 数值（伤害量/治疗量等） |
| `currentHp` | `int` | 触发者当前HP |
| `currentQi` | `int` | 触发者当前气力 |
| `maxHp` | `int` | 触发者最大HP |
| `attack` | `int` | 触发者当前攻击力（含Buff） |
| `defense` | `int` | 触发者当前防御力（含Buff） |
| `speed` | `int` | 触发者当前速度（含Buff） |
| `message` | `std::string` | 可读描述文本 |

**事件类型枚举 `BattleEventType`**：

| 枚举值 | 含义 |
|---|---|
| `DamageTaken` | 受到伤害 |
| `Healed` | 受到治疗 |
| `QiConsumed` | 消耗气力 |
| `QiRestored` | 恢复气力 |
| `Died` | 角色死亡 |
| `TurnStart` | 回合开始 |
| `Attack` | 发动攻击 |
| `BattleWin` | 战斗胜利 |
| `BattleLose` | 战斗失败 |
| `BattleStart` | 战斗开始 |
| `BuffApplied` | Buff施加成功 |
| `BuffExpired` | Buff到期消失 |
| `DotTick` | 持续伤害/治疗触发 |
| `StatChanged` | 属性发生变化（Buff影响） |

### 4.4 战斗驱动

```cpp
session.Start();                          // 开始战斗
while (!session.IsOver()) {
    session.Update();                     // 推进一帧（ATB行动条+AI决策）

    // 如果是玩家回合，根据当前阶段收集输入
    if (session.GetPhase() == GameController::Phase::PlayerMainMenu) {
        session.SubmitMainAction(MainActionType::Skill);  // 选择主行动：攻击/技能/回气
    }
    else if (session.GetPhase() == GameController::Phase::PlayerSkillMenu) {
        session.SubmitSkill(0);           // 选择第0个技能，-1表示返回上级
    }
    else if (session.GetPhase() == GameController::Phase::PlayerTargetSelect) {
        session.SubmitTarget(0);          // 选择第0个目标，-2表示返回上级
    }
}

auto result = session.GetResult();
if (result.playerWon) { /* 胜利处理 */ }
```

| 方法 | 说明 |
|---|---|
| `void Start()` | 初始化行动条并正式开始战斗 |
| `void Update()` | 推进一帧。内部会自动推进ATB条、处理敌人AI回合、等待玩家输入阶段不做任何事 |
| `bool IsOver() const` | 战斗是否已结束 |
| `BattleResult GetResult() const` | 获取战斗结果 |

**`BattleResult` 结构**：

| 字段 | 类型 | 说明 |
|---|---|---|
| `isOver` | `bool` | 是否结束 |
| `playerWon` | `bool` | 我方是否胜利 |

### 4.5 玩家输入提交

| 方法 | 参数 | 说明 |
|---|---|---|
| `void SubmitMainAction(MainActionType action)` | `Attack` / `Skill` / `RestoreQi` | 在 `PlayerMainMenu` 阶段提交主行动选择 |
| `void SubmitSkill(int skillIndex)` | 技能列表中的索引，`-1` 表示返回上级 | 在 `PlayerSkillMenu` 阶段提交技能选择 |
| `void SubmitTarget(int targetIndex)` | 目标候选列表中的索引，`-2` 表示返回上级 | 在 `PlayerTargetSelect` 阶段提交目标选择 |

**`MainActionType` 枚举**：

| 值 | 含义 |
|---|---|
| `Attack` | 普通攻击（进入目标选择） |
| `Skill` | 使用技能（进入技能列表） |
| `RestoreQi` | 回气（无需目标，立即执行） |

### 4.6 状态查询（用于UI展示）

| 方法 | 返回值 | 说明 |
|---|---|---|
| `GameController::Phase GetPhase() const` | 当前阶段枚举 | 判断当前处于哪个输入阶段 |
| `Character* GetCurrentActor() const` | 当前行动角色指针 | 当前回合的角色 |
| `std::vector<Character*> GetPlayerTeam() const` | 我方队伍 | 含完整状态（HP/气力/Buff等） |
| `std::vector<Character*> GetEnemyTeam() const` | 敌方队伍 | 含完整状态 |
| `std::vector<std::pair<std::string, float>> GetActionOrder() const` | (角色名, 预计行动时间) 列表 | 用于ATB条UI排序展示 |
| `std::vector<Character*> GetTargetCandidates() const` | 当前可选目标 | 根据技能目标类型自动筛选 |
| `TargetType GetExpectedTargetType() const` | 目标类型枚举 | 当前需要选择的目标类型 |
| `std::vector<std::string> FlushLogs()` | 字符串列表 | 获取并清空内部日志缓冲 |

**`GameController::Phase` 枚举**：

| 值 | 含义 |
|---|---|
| `Idle` | 空闲/ATB推进中 |
| `PlayerMainMenu` | 等待玩家选择主行动 |
| `PlayerSkillMenu` | 等待玩家选择技能 |
| `PlayerTargetSelect` | 等待玩家选择目标 |
| `Executing` | 执行动画/结算中 |
| `EnemyTurn` | 敌人AI回合 |
| `BattleOver` | 战斗结束 |

**`TargetType` 枚举**：

| 值 | 含义 |
|---|---|
| `Enemy` | 敌方目标 |
| `Ally` | 己方目标 |
| `Self` | 自身 |
| `None` | 无需目标 |

---

## 五、战斗 UI：`BattleConsoleUI`

如果不想手动处理 `DisplayCallback` 和输入驱动，可直接使用 `BattleConsoleUI` 一站式完成战斗渲染和交互。

### 5.1 接口速查

| 方法 | 说明 |
|---|---|
| `static void Bind(BattleSession& session)` | 绑定事件回调到 UI 内部日志收集器 |
| `static void ClearLogs()` | 清空内部战斗日志缓存 |
| `static BattleOutcome RunManualBattle(BattleSession& session)` | 手动战斗（渲染+输入+驱动） |
| `static BattleOutcome RunManualBattle(GameManager& gm)` | 同上，通过 GameManager，战后自动结算 |
| `static void RunAutoBattle(BattleSession& session)` | 自动战斗（AI 选攻击） |

**`BattleOutcome` 枚举**：

| 值 | 含义 |
|---|---|
| `Victory` | 战斗胜利 |
| `Defeat` | 战斗失败 |
| `Fled` | 玩家逃跑 |

### 5.2 渲染画面说明

`RunManualBattle` 每轮玩家回合会：

1. 清屏
2. 绘制行动顺序（前5名）
3. 绘制敌方/我方队伍面板（含 Buff 状态）
4. 绘制最近 20 条战斗日志
5. 根据当前阶段弹出菜单并读取输入

---

## 六、完整使用示例

### 6.1 手动战斗（使用 BattleConsoleUI）

```cpp
#include "core/battle/BattleSession.h"
#include "core/battle/BattleConfigLoader.h"
#include "core/ui/BattleConsoleUI.h"

// 1. 加载配置
SkillConfigLoader::LoadFromFile("assets/config/skills.json");
EnemyConfigLoader::LoadFromFile("assets/config/enemies.json");

// 2. 创建会话
BattleSession session;
int p1 = session.AddPlayer("李逍遥", 120, 60, 12, 25, 8);
session.AddPlayerSkill(p1, "slash");
session.AddPlayerSkill(p1, "fireball");
session.AddEnemyGroup("novice");

// 3. 运行手动战斗（UI 自动处理显示和输入）
BattleOutcome outcome = BattleConsoleUI::RunManualBattle(session);
std::cout << (outcome == BattleOutcome::Victory ? "胜利" : "失败/逃跑") << "\n";
```

### 6.2 自动战斗（测试用）

```cpp
BattleSession session;
session.AddPlayer("李逍遥", 200, 80, 15, 30, 10);
session.AddEnemyGroup("boss");
BattleConsoleUI::RunAutoBattle(session);
auto result = session.GetResult();
std::cout << (result.playerWon ? "胜利" : "失败") << "\n";
```

### 6.3 手动驱动（不使用 UI 模块）

```cpp
BattleSession session;
session.SetDisplayCallback([](const BattleGameEvent& e) {
    std::cout << "[" << e.characterName << "] " << e.message << "\n";
});

int p1 = session.AddPlayer("李逍遥", 200, 80, 15, 30, 10);
session.AddEnemyGroup("novice");
session.Start();

while (!session.IsOver()) {
    session.Update();
    auto phase = session.GetPhase();
    if (phase == GameController::Phase::PlayerMainMenu) {
        session.SubmitMainAction(MainActionType::Attack);
    }
    else if (phase == GameController::Phase::PlayerTargetSelect) {
        session.SubmitTarget(0);
    }
}
```

---

## 七、注意事项

1. **技能/敌人ID必须与配置表一致**，否则内部会找不到对应定义。
2. **每局战斗需新建 `BattleSession`**，该类不支持重置复用。
3. **敌人AI是内置的随机决策**（攻击/技能/回气三选一），外部无法干预。
4. **无持久化接口**：战斗是一次性过程，不涉及 `Serialize`/`Deserialize`。
5. **线程不安全**：所有操作应在单线程中顺序执行。
6. **`TurnManager` 保留在代码中但当前未活跃使用**：实际战斗驱动由 `GameController` 内的 `ActionGaugeSystem` 负责。
