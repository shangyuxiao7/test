# 接口文档编写规范

> **用途**：供 AI 编码助手或其他开发者参考，确保新增/修改模块的接口文档风格与本项目现有文档保持一致。  
> **适用范围**：`docs/interfaces/` 目录下的所有模块接口文档。

---

## 一、文档定位

接口文档是**面向使用者的说明书**，不是代码注释的搬运。目标是让阅读者**只看文档就能正确调用接口**，不需要翻源代码。

- **不要**罗列每个私有成员和内部实现细节
- **不要**只写函数签名，不写使用场景
- **要**写清楚：先做什么、再做什么、输入什么、得到什么、会失败吗

---

## 二、文件与命名

| 规则 | 说明 |
|---|---|
| 文件名 | `{模块英文名}Interface.md`，如 `BattleInterface.md`、`InventoryInterface.md` |
| 存放位置 | `docs/interfaces/` |
| 编码 | UTF-8，带 BOM（与项目源文件保持一致） |

---

## 三、文档结构模板

所有接口文档必须按以下顺序组织章节。如果某章节不适用（如该模块无配置加载），可跳过，但**不要调换顺序**。

```markdown
# {模块中文名}接口文档 ({Module Name} Module)

> **对外接口类**: `ClassA`（职责）, `ClassB`（职责）  
> **配套 UI**: `GameConsoleUI::Xxx`（如有）  
> **模块路径**: `core/xxx/`  
> **配置路径**: `assets/config/xxx.json`（如有）

---

## 一、模块概述

- 一句话定义模块职责
- 列出 3-5 个核心功能点
- **设计原则**（如零 I/O、效果零执行、PIMPL 封装等）
- **与其他模块的关系**（如"装备实际物品管理由背包系统负责"）

---

## 二、依赖关系树状图

用 ASCII 字符画展示模块内部层级依赖，格式如下：

```
                              main / GameManager
                                     │
                          WorldFacade（唯一对外接口，PIMPL）
                                     │
                          WorldManager
                                     │
                          ┌──────────┴──────────┐
                          │                     │
                      MapInstance           WorldEvent
                          │
                    ┌─────┴─────┐
                    │           │
                RoomGraph     Entity
```

绘制规范：
- 最顶层统一为 `main / GameManager`
- 对外接口类放在第二层，标注是否"唯一对外接口"或"PIMPL"
- 底层配置/数据结构放在最下方
- 用 `│`、`┌`、`┐`、`┘`、`└`、`─` 连接，不要用 tab 缩进混用空格

---

## 三、配置加载

如果模块需要 JSON 配置，写明：
- 加载代码示例（含 `#include` 路径）
- 每个配置加载器的**方法表格**（类、方法、说明）

---

## 四、数据结构

把模块暴露的**纯数据结构**放在核心接口之前，避免读者在接口表格里反复看到陌生类型。

每个数据结构一个三级标题，内容用**表格**：

```markdown
### 4.1 `EquipmentSlot`（装备槽位枚举）

| 值 | 说明 |
|---|---|
| `Weapon` | 武器 |
```

需要覆盖的数据结构类型：
- 枚举（标注定义所在头文件，如果不在本模块内）
- struct / 配置定义（每个字段：名称、类型、说明）
- 全局单例查询类（给出典型调用示例）

---

## 五、核心接口类

### 5.1 标题命名

每个对外接口类一个二级标题：

```markdown
## 五、核心接口类：`PartyManager`
```

### 5.2 功能分组

方法按功能分组，用三级标题：
- 构造与生命周期
- 队伍管理 / 查询接口 / 修改接口
- 战斗数据导出
- 战后同步
- 存档接口

### 5.3 方法表格

每个方法必须用**表格**呈现，不要只写列表：

| 方法 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `int AddMember(const std::string& id, int level = 1)` | 角色ID、等级 | 成员索引 | 添加成员。若配置未加载，属性会初始化为0 |

表格规范：
- **参数**列：类型+名称，默认参数写 `= xxx`
- **返回值**列：类型+含义。失败时返回什么（如 `nullptr` / `-1` / `false`）必须写
- **说明**列：写清失败原因、副作用、前置条件。不要只写"添加成员"

### 5.4 代码示例

每个功能组下面紧跟**最小可运行示例**：

```cpp
int idx = party.AddMember("li_xiaoyao", 5);   // 添加李逍遥，等级5
party.RemoveMember(0);                          // 移除索引0的成员
```

示例规范：
- 必须写注释说明每行在做什么
- 使用实际项目中的 ID（如 `"li_xiaoyao"`、`"heal_herb"`），不要用 `foo`、`bar`
- 如果涉及回调，给出 lambda 示例

---

## 六、UI 显示

如果该模块有配套的 `GameConsoleUI` 方法，单独设立此章节：

```markdown
## 六、UI 显示

推荐使用 `GameConsoleUI::ShowParty` 显示队伍信息：

```cpp
GameConsoleUI::ShowParty(party);
```

输出示例：
```
--- 当前队伍 ---
  [1] li_xiaoyao Lv.1  HP:150/150  QI:50/50  ATK:15  DEF:8  SPD:10 [参战]
----------------
```
```

要求：
- 给出实际调用代码
- 给出实际输出样例（帮助阅读者预期效果）

---

## 七、完整使用示例

提供一个**端到端**的完整代码片段，覆盖该模块的典型使用流程。

```cpp
#include "core/character/PartyManager.h"
#include "core/character/CharacterConfigLoader.h"
#include "core/ui/GameConsoleUI.h"

// 1. 统一加载配置
CharacterConfigLoader::LoadAllFromDirectory("assets/config");

// 2. 创建队伍
PartyManager party;
party.AddMember("li_xiaoyao", 1);

// 3. 养成操作
auto* m = party.GetMember(0);
m->GainExp(150);
m->Equip(EquipmentSlot::Weapon, "iron_sword");

// 4. 显示
GameConsoleUI::ShowParty(party);

// 5. 导出战斗数据
auto battleData = party.ExportActiveMembersForBattle();

// 6. 战后同步
std::vector<BattleMemberState> results;
results.push_back({"li_xiaoyao", 80, 20, false, 200});
party.SyncAfterBattle(results);

// 7. 存档
std::string save;
party.Serialize(save);
```

完整示例规范：
- 带步骤编号（1. 2. 3.），每步一行注释
- 包含配置加载、业务操作、UI 显示、数据导出/同步、存档
- 使用实际项目中的真实 ID 和文件名

---

## 八、注意事项

用条目列表写清楚使用该模块时必须注意的限制和陷阱：

1. **必须先加载配置再创建成员**：`AddMember` 依赖 `characters.json`...
2. **装备加成内部计算**：`GetAttack()` 已自动包含装备加成...
3. **线程不安全**：所有操作应在单线程中顺序执行。

每条注意事项的格式：
- 先加粗总结（如 **必须先加载配置**）
- 再用普通文字解释原因和后果

---

## 九、风格检查清单

写完文档后，对照以下清单自查：

- [ ] 标题包含模块中文名和英文名
- [ ] 元信息行包含对外接口类、模块路径、配置路径
- [ ] 有依赖关系树状图，且最顶层是 `main / GameManager`
- [ ] 所有方法用表格呈现（参数、返回值、说明三列齐全）
- [ ] 返回值列写明了失败时的返回（如 `nullptr` / `-1` / `false`）
- [ ] 有可直接运行的完整使用示例，使用项目真实 ID
- [ ] 有配套 UI 方法的说明和输出样例（如有）
- [ ] 注意事项不少于 3 条
- [ ] 全文没有出现 "foo"、"bar"、"test123" 等占位符
- [ ] 代码块标注了语言（`cpp`）

---

## 十、参考范例

本项目已完成的接口文档可作为直接参考：

| 文档 | 特点 |
|---|---|
| `BattleInterface.md` | 包含两套示例（用 UI / 不用 UI）、有战斗 UI 专门章节 |
| `CharacterInterface.md` | 多数据结构（枚举、struct、单例）、单角色操作独立成章 |
| `InventoryInterface.md` | 强调"效果零执行"设计原则、EventBus 事件机制 |
| `WorldInterface.md` | PIMPL 封装说明、条件出口与全局标记系统 |
| `UIInterface.md` | 静态类接口、渲染行为详细说明 |
