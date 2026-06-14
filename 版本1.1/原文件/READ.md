依赖关系树状图

                                main
                                  │
                          GameController
                ┌───────────┼───────────┐
                │           │           │
            ConsoleUI    BattleManager  ActionGaugeSystem
                │           │           │
                │           │           └──────────► Character
                │           │
                │           └── TurnManager（保留但不活跃）
                │               │
                │               └── Character
                │                   │
                │       ┌─────┼─────┐
                │       │          │          │
                │   Attributes   GameEvent  StatusEffect
                │       │          │          │
                │       └─────┬─────┘           │
                │             │                 │
                │         ICombatant           │
                │             │                 │
                │             └───────┬───────┘
                │                     │
                │                   Action
                │                     │
                │     ┌───────┼────────┐
                │     │               │               │
                │ NormalAttack   DamageSkill      HealSkill
                │     │               │               │
                │     └───────┼────────┘
                │                     │
                │             ┌───┴───────┐
                │             │               │
                │         BuffSkill    CompositeSkill
                │             ▲               │
                │             │               │
                │     SkillLibrary             │
                │             │               │
                │             └───┬───────┘
                │                     │
                │               Action 派生类
                │
                └──────────┘
                                      │
                                Character 继承链
                                      │
                          ┌─────┴─────┐
                          │                      │
                    PlayerCharacter            AICharacter


**连线说明**：
- **竖线 `│` 和横线 `─`**：表示依赖关系（上层依赖下层，或包含关系）。
- **`───►`**：表示运行时依赖或关联关系。`ActionGaugeSystem` 通过 `Character*` 指针操作行动条进度；`EnemyFactory` 创建 `AICharacter` 并装配技能（图中省略连线，以文字标注）。
- **`▲`**：表示继承关系。`BuffSkill` / `CompositeSkill` 继承自 `Action`；`SkillLibrary` 返回 `Action*` 实例。
- **最底层**：`Attributes`、`GameEvent`、`StatusEffect` 不依赖任何项目内其他类。
- **`TurnManager`**：保留在 `BattleManager` 内部，但 `GameController` 不再调用（当前活跃架构中不参与战斗流程）。

---

## 类/结构体职责清单

| 类/结构体 | 所在文件 | 职责 |
|-----------|----------|------|
| `main` | `pal_20260405.cpp` | 程序入口，创建角色、注册技能、组装队伍、驱动 `Tick` + `Render` 非阻塞主循环。 |
| `GameController` | `GameController.h` | **战斗状态机总控**，内含行动条系统 `ActionGaugeSystem`。管理 `Idle → 玩家回合 → 敌方回合 → 执行` 的状态流转，**零 I/O**。 |
| `ConsoleUI` | `ConsoleUI.h` | **控制台显示层**，根据 `GameController` 阶段渲染战场状态、接收 `cin` 输入并调用 `SubmitXxx()`。**换平台时直接替换**。 |
| `BattleManager` | `BattleManager.h` | 队伍管理器，维护敌我双方角色列表，胜负判定，战斗结束后的内存清理。 |
| `ActionGaugeSystem` | `GameController.h`（嵌套结构） | 行动条核心引擎，推进抽象时间、收集同时到达角色、按速度排序、重置进度。运行时通过 `Character*` 指针操作，**`Character` 类零感知**。 |
| `TurnManager` | `TurnManager.h` | 旧版轮次管理器（保留在项目中但不再被 `GameController` 调用，仅作向后兼容）。 |
| `Character` | `Character.h` / `Character.cpp` | 角色抽象基类，管理属性、状态效果列表、临时攻防修正、事件通知、回合接口。 |
| `PlayerCharacter` | `PlayerCharacter.h` | 玩家控制角色，继承 `Character`。 |
| `AICharacter` | `AICharacter.h` | AI 控制角色，继承 `Character`，随机选择目标。 |
| `ICombatant` | `ICombatant.h` | 战斗单位抽象接口，定义属性查询、伤害、治疗、气力、状态施加等契约。 |
| `Action` | `Action.h` | 行为抽象基类，自描述技能规则（名称、气力消耗、目标类型、可用性判断）。 |
| `NormalAttack` | `Action.h` | 普通攻击，继承 `Action`。 |
| `RestoreQiAction` | `Action.h` | 回蓝行为，无需目标，继承 `Action`。 |
| `DamageSkill` | `Action.h` | 伤害技能，消耗气力造成固定伤害，继承 `Action`。 |
| `HealSkill` | `Action.h` | 治疗技能，目标类型为友方，继承 `Action`。 |
| `BuffSkill` | `Action.h` | Buff / Debuff 技能，对目标施加 `StatusEffect`，继承 `Action`。 |
| `CompositeSkill` | `Action.h` | 组合技能（伤害 + 附加效果），继承 `Action`。 |
| `SkillLibrary` | `SkillLibrary.h` | 技能工厂，预制并返回各类 `Action` 派生对象。 |
| `EnemyFactory` | `EnemyFactory.h` | 敌人预制体工厂，创建带技能的 `AICharacter`，提供单体敌人与波次群组。 |
| `Attributes` | `Attributes.h` | 纯数据容器，存储角色基础生命、气力、速度、攻击、防御。 |
| `GameEvent` | `GameEvent.h` | 事件数据结构，承载计算层到显示层的所有消息（伤害、Buff、死亡等）。 |
| `DisplayCallback` | `GameEvent.h` | 显示回调函数类型，`std::function<void(const GameEvent&)>`。 |
| `StatusEffect` | `StatusEffect.h` | 状态效果数据结构，定义 Buff / DoT / HoT 的名称、类型、数值、持续回合。 |

---