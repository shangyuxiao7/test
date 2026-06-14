# 角色模块接口文档 (Character Module)

> **对外接口类**: `PartyManager`（队伍管理）, `PartyMember`（单角色操作）  
> **配套 UI**: `GameConsoleUI::ShowParty`（`core/ui/GameConsoleUI.h`）  
> **模块路径**: `core/character/`  
> **配置路径**: `assets/config/characters.json`, `assets/config/equipments.json`, `assets/config/curves.json`

---

## 一、模块概述

角色养成与队伍管理系统。职责：

- 队伍成员的增删改查、活跃/待命切换
- 角色升级、经验计算、属性成长（通过曲线表）
- 装备穿戴/卸下（属性加成计算在模块内部完成）
- 向战斗系统导出角色初始数据（`CombatantInitData`）
- 战后状态同步（HP/气力/经验/死亡状态）

**与背包模块的关系**：装备的实际物品管理由背包系统负责，角色模块只记录 `"穿了哪件装备"`（装备ID），并通过 `CharacterDataTables` 查询装备属性加成。

---

## 二、依赖关系树状图

```
                              main / GameManager
                                     │
                    ┌────────────────┼────────────────┐
                    │                │                │
              GameConsoleUI    PartyManager（对外接口）  InventoryManager
                    │                │                │
              ShowParty          PartyMember        EquipItem
                    │                │                │
         CharacterDataTables    GrowthCurve      EquipmentSlot
                   │                 │              (inventory/Types.h)
                   │                 │
                   └─────────────────┴─────────────────┘
                                       │
                            CharacterConfigLoader
                                       │
                            ┌──────────┼──────────┐
                            │          │          │
                      characters.json equipments.json curves.json
```

---

## 三、配置加载

```cpp
#include "core/character/CharacterConfigLoader.h"

// 统一入口：一行加载角色、装备、曲线三个配置
CharacterConfigLoader::LoadAllFromDirectory("assets/config");
```

| 类 | 方法 | 说明 |
|---|---|---|
| `CharacterConfigLoader` | `static bool LoadAllFromDirectory(const std::string& dir)` | 统一入口，加载 dir/characters.json、equipments.json、curves.json |
| `CharacterConfigLoader` | `static bool LoadCharactersFromFile(const std::string& filepath)` | 单独加载角色定义 |
| `CharacterConfigLoader` | `static bool LoadEquipmentsFromFile(const std::string& filepath)` | 单独加载装备定义 |
| `CharacterConfigLoader` | `static bool LoadCurvesFromFile(const std::string& filepath)` | 单独加载成长曲线定义 |

---

## 四、数据结构

### 4.1 `EquipmentSlot`（装备槽位枚举）

> 定义在 `core/inventory/Types.h`

| 值 | 说明 |
|---|---|
| `Weapon` | 武器 |
| `Armor` | 防具 |
| `Shoes` | 鞋子 |
| `Accessory` | 饰品/宝物 |

### 4.2 `CharacterDef`（角色定义，只读配置数据）

| 字段 | 类型 | 说明 |
|---|---|---|
| `id` | `std::string` | 唯一ID（如 `"li_xiaoyao"`） |
| `name` | `std::string` | 显示名称（如 `"李逍遥"`） |
| `baseHp` | `int` | 基础生命 |
| `baseQi` | `int` | 基础气力 |
| `baseSpeed` | `int` | 基础速度 |
| `baseAttack` | `int` | 基础攻击 |
| `baseDefense` | `int` | 基础防御 |
| `initialSkills` | `std::vector<std::string>` | 初始技能ID列表 |
| `growthCurveId` | `std::string` | 关联的成长曲线ID |

### 4.3 `EquipmentDef`（装备定义，只读配置数据）

| 字段 | 类型 | 说明 |
|---|---|---|
| `id` | `std::string` | 唯一ID（如 `"iron_sword"`） |
| `name` | `std::string` | 显示名称 |
| `slot` | `EquipmentSlot` | 装备槽位 |
| `bonusAttack` | `int` | 攻击加成 |
| `bonusDefense` | `int` | 防御加成 |
| `bonusHp` | `int` | 生命加成 |
| `bonusQi` | `int` | 气力加成 |
| `bonusSpeed` | `int` | 速度加成 |

### 4.4 `CharacterDataTables`（全局配置查询单例）

```cpp
CharacterDataTables::Instance().GetCharacterDef("li_xiaoyao");   // 返回 const CharacterDef* 或 nullptr
CharacterDataTables::Instance().GetEquipmentDef("iron_sword");   // 返回 const EquipmentDef* 或 nullptr
CharacterDataTables::Instance().GetAllCharacterDefs();
CharacterDataTables::Instance().GetAllEquipmentDefs();
CharacterDataTables::Instance().ClearCharacters();
CharacterDataTables::Instance().ClearEquipments();
```

### 4.5 `CombatantInitData`（导出给战斗系统的纯数据结构）

| 字段 | 类型 | 说明 |
|---|---|---|
| `characterId` | `std::string` | 配置表中的角色唯一ID |
| `name` | `std::string` | 角色显示名 |
| `level` | `int` | 等级 |
| `hp` | `int` | 当前HP（导出时 = maxHp） |
| `maxHp` | `int` | 最大HP（含装备加成） |
| `qi` | `int` | 当前气力（导出时 = maxQi） |
| `maxQi` | `int` | 最大气力（含装备加成） |
| `speed` | `int` | 速度（含装备加成） |
| `attack` | `int` | 攻击（含装备加成） |
| `defense` | `int` | 防御（含装备加成） |
| `skillIds` | `std::vector<std::string>` | 可用技能ID列表 |

### 4.6 `BattleMemberState`（战后同步数据，由战斗系统传入）

| 字段 | 类型 | 说明 |
|---|---|---|
| `characterId` | `std::string` | 角色ID |
| `remainingHp` | `int` | 战后剩余HP |
| `remainingQi` | `int` | 战后剩余气力 |
| `isDead` | `bool` | 是否死亡 |
| `expGained` | `int` | 获得的经验值 |

---

## 五、核心接口类：`PartyManager`

### 5.1 构造

```cpp
#include "core/character/PartyManager.h"

PartyManager party;
```

| 方法 | 说明 |
|---|---|
| `PartyManager()` | 默认构造 |
| `~PartyManager()` | 默认析构 |

### 5.2 队伍管理

```cpp
int idx = party.AddMember("li_xiaoyao", 5);   // 添加李逍遥，等级5
party.RemoveMember(0);                          // 移除索引0的成员
party.SetMemberActive(0, false);                // 把索引0设为待命
party.SwapMemberOrder(0, 1);                    // 交换索引0和1的位置
```

| 方法 | 参数 | 返回值 | 说明 |
|---|---|---|---|
| `int AddMember(const std::string& characterId, int level = 1)` | 角色ID、等级 | 成员索引（从0开始） | 添加成员。若配置未加载，属性会初始化为0 |
| `void RemoveMember(int index)` | 索引 | - | 移除指定成员 |
| `void SetMemberActive(int index, bool active)` | 索引、是否活跃 | - | 设置成员活跃/待命状态 |
| `void SwapMemberOrder(int indexA, int indexB)` | 两个索引 | - | 交换两名成员的位置 |

### 5.3 查询接口

| 方法 | 返回值 | 说明 |
|---|---|---|
| `PartyMember* GetMember(int index)` | 角色指针 | 获取指定索引的成员（非 const） |
| `int GetMemberCount() const` | 数量 | 当前队伍总人数 |
| `std::vector<PartyMember*> GetActiveMembers() const` | 活跃成员列表 | 仅返回 `IsActive()==true` 且未死亡的成员 |

### 5.4 战斗数据导出

```cpp
// 导出所有活跃成员，用于战斗系统初始化
std::vector<CombatantInitData> initData = party.ExportActiveMembersForBattle();
```

| 方法 | 返回值 | 说明 |
|---|---|---|
| `std::vector<CombatantInitData> ExportActiveMembersForBattle() const` | 初始化数据列表 | 导出活跃成员的战斗初始数据（含装备加成后的属性） |

### 5.5 战后同步

```cpp
// 战斗结束后，把战斗结果同步回角色系统
std::vector<BattleMemberState> results = /* 来自战斗系统 */;
party.SyncAfterBattle(results);
```

| 方法 | 参数 | 说明 |
|---|---|---|
| `void SyncAfterBattle(const std::vector<BattleMemberState>& results)` | 战后状态列表 | 同步HP/气力/死亡状态/经验值，经验满足条件会自动升级 |

### 5.6 存档接口

```cpp
std::string saveData;
party.Serialize(saveData);     // 导出为字符串

PartyManager party2;
party2.Deserialize(saveData);  // 从字符串恢复（含等级/经验/装备/活跃状态）
```

| 方法 | 说明 |
|---|---|
| `void Serialize(std::string& out) const override` | 序列化为字符串 |
| `void Deserialize(const std::string& in) override` | 从字符串反序列化 |

---

## 六、单角色操作：`PartyMember`

通过 `party.GetMember(index)` 获取指针后操作：

### 6.1 养成操作

```cpp
auto* m = party.GetMember(0);
m->GainExp(150);                              // 获得150经验，可能自动升级
m->Equip(EquipmentSlot::Weapon, "iron_sword"); // 装备武器
m->Unequip(EquipmentSlot::Weapon);            // 卸下武器
m->SetActive(false);                          // 设为待命（仍保留在队伍中）
m->SetDead(true);                             // 标记死亡（战斗中死亡后）
m->LearnSkill("new_skill_001");               // 学习新技能
```

| 方法 | 参数 | 说明 |
|---|---|---|
| `void GainExp(int amount)` | 经验值 | 增加经验，满足升级条件时自动升级并刷新属性 |
| `void Equip(EquipmentSlot slot, const std::string& equipmentId)` | 槽位、装备ID | 穿戴装备。会覆盖同槽位已有装备 |
| `void Unequip(EquipmentSlot slot)` | 槽位 | 卸下指定槽位的装备 |
| `void SetActive(bool active)` | 是否活跃 | 设置活跃/待命 |
| `void SetDead(bool dead)` | 是否死亡 | 设置死亡状态 |
| `void LearnSkill(const std::string& skillId)` | 技能ID | 学习新技能 |

### 6.2 查询接口

| 方法 | 返回值 | 说明 |
|---|---|---|
| `int GetLevel() const` | 等级 | 当前等级 |
| `int GetCurrentHp() const` | HP | 当前生命值 |
| `int GetMaxHp() const` | 最大HP | 最大生命值（含装备加成） |
| `int GetCurrentQi() const` | 气力 | 当前气力值 |
| `int GetMaxQi() const` | 最大气力 | 最大气力值（含装备加成） |
| `int GetAttack() const` | 攻击力 | 含装备加成 |
| `int GetDefense() const` | 防御力 | 含装备加成 |
| `int GetSpeed() const` | 速度 | 含装备加成 |
| `bool IsActive() const` | 是否活跃 | 待命返回 false |
| `bool IsDead() const` | 是否死亡 | 死亡返回 true |
| `std::vector<std::string> GetSkillIds() const` | 技能ID列表 | 当前掌握的所有技能 |
| `std::string GetEquippedItem(EquipmentSlot slot) const` | 装备ID | 指定槽位的装备ID，空槽返回 `""` |
| `std::string GetCharacterId() const` | 角色ID | 配置表中的角色ID |
| `int GetCurrentExp() const` | 当前经验 | 当前等级已积累的经验 |
| `int GetExpToNextLevel() const` | 还需经验 | 升到下一级还差多少经验 |

### 6.3 战后同步（单角色）

```cpp
m->SyncAfterBattle(remainingHp, remainingQi, isDead);
```

| 方法 | 参数 | 说明 |
|---|---|---|
| `void SyncAfterBattle(int remainingHp, int remainingQi, bool isDead)` | 战后剩余HP/气力/死亡状态 | 同步战斗后的即时状态 |

### 6.4 导出战斗数据（单角色）

```cpp
CombatantInitData data = m->ExportForBattle();
```

| 方法 | 返回值 | 说明 |
|---|---|---|
| `CombatantInitData ExportForBattle() const` | 战斗初始化数据 | 导出该角色的完整战斗初始数据 |

---

## 七、UI 显示

推荐使用 `GameConsoleUI::ShowParty` 显示队伍信息，无需手动拼接字符串：

```cpp
#include "core/ui/GameConsoleUI.h"

PartyManager party;
party.AddMember("li_xiaoyao", 1);
GameConsoleUI::ShowParty(party);
```

输出示例：
```
--- 当前队伍 ---
  [1] li_xiaoyao Lv.1  HP:150/150  QI:50/50  ATK:15  DEF:8  SPD:10 [参战]
----------------
```

---

## 八、完整使用示例

```cpp
#include "core/character/PartyManager.h"
#include "core/character/CharacterConfigLoader.h"
#include "core/ui/GameConsoleUI.h"

// 1. 统一加载配置
CharacterConfigLoader::LoadAllFromDirectory("assets/config");

// 2. 创建队伍
PartyManager party;
party.AddMember("li_xiaoyao", 1);
party.AddMember("zhao_linger", 1);

// 3. 养成操作
auto* li = party.GetMember(0);
li->GainExp(150);                              // 可能升级到 Lv.2
li->Equip(EquipmentSlot::Weapon, "iron_sword"); // +5 攻击

// 4. 显示队伍
GameConsoleUI::ShowParty(party);

// 5. 导出战斗数据（只导出活跃成员）
auto battleData = party.ExportActiveMembersForBattle();
for (const auto& d : battleData) {
    std::cout << d.name << " Lv." << d.level
              << " HP:" << d.hp << "/" << d.maxHp
              << " ATK:" << d.attack << "\n";
}

// 6. 战后同步（假设战斗系统返回结果）
std::vector<BattleMemberState> results;
results.push_back({"li_xiaoyao", 80, 20, false, 200});
party.SyncAfterBattle(results);

// 7. 存档
std::string save;
party.Serialize(save);
```

---

## 九、注意事项

1. **必须先加载配置再创建成员**：`AddMember` 依赖 `characters.json`，`Equip` 依赖 `equipments.json`，升级依赖 `curves.json`。推荐统一使用 `LoadAllFromDirectory`。
2. **装备加成内部计算**：`GetAttack()` / `GetDefense()` / `GetMaxHp()` 等已自动包含装备加成，外部无需手动累加。
3. **`ExportActiveMembersForBattle` 只导出活跃成员**：待命或死亡成员不会出现在返回列表中。
4. **战后经验自动升级**：`SyncAfterBattle` 传入的 `expGained` 会调用 `GainExp`，满足条件时自动升级。
5. **`GetMember` 返回非 const 指针**：因为需要对角色进行操作（装备、升级等）。
6. **线程不安全**：所有操作应在单线程中顺序执行。
