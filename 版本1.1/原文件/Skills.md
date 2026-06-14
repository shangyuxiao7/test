Markdown
复制
代码
预览
# 回合制战斗游戏 - 技能系统开发文档

## 1. 技能系统概述

当前技能系统基于 `Action` 抽象基类，所有可执行行为（普通攻击、回蓝、伤害技能、治疗技能、Buff、组合技能）均继承自 `Action`。`Character` 拥有独立的 `skills` 列表（与 `actions` 分离），通过 `AddSkill()` 注册。

核心设计原则：
- **自描述**：每个技能自带 `IsAvailable()`（是否可用）、`NeedTarget()`（是否需要目标）、`GetTargetType()`（目标规则）
- **统一接口**：所有技能通过 `Execute(ICombatant* user, ICombatant* target)` 执行
- **状态效果**：`StatusEffect` 模块支持 Buff/Debuff/DoT/HoT，由 `Character::UpdateEffects()` 每轮结算

---

## 2. 现有技能类清单

### 2.1 Action 基类（Action.h）
```cpp
class Action {
protected:
    std::string name;
    int qiCost;
public:
    virtual void Execute(ICombatant* user, ICombatant* target) = 0;
    virtual bool IsAvailable(const ICombatant* user) const;  // 默认检查 qi >= qiCost
    virtual bool NeedTarget() const { return true; }
    virtual TargetType GetTargetType() const { return TargetType::Enemy; }
};
2.2 目标类型枚举（Action.h）
cpp
复制
enum class TargetType {
    Enemy,   // 敌方目标
    Ally,    // 友方目标
    Self,    // 自身
    None     // 无需目标
};
2.3 已有派生类
NormalAttack - 普通攻击
消耗：0 气
目标：Enemy
效果：调用 user->Attack(target)（受攻击方防御减免）
RestoreQiAction - 回蓝
消耗：0 气
目标：None（NeedTarget 返回 false）
效果：user->RestoreQi(amount)
DamageSkill - 伤害技能
构造：DamageSkill(name, qiCost, damage)
目标：Enemy
效果：先 ConsumeQi，再 target->TakeDamage(damage)（固定值，不受攻击buff影响）
HealSkill - 治疗技能
构造：HealSkill(name, qiCost, healAmount)
目标：Ally
效果：先 ConsumeQi，再 target->Heal(amount)
BuffSkill - 状态效果技能
构造：BuffSkill(name, qiCost, StatusEffect, TargetType)
效果：先 ConsumeQi，再 target->ApplyStatus(effect)
注意：目标类型可自定义（Enemy/Ally/Self）
CompositeSkill - 组合技能
构造：CompositeSkill(name, qiCost, damage, StatusEffect, TargetType)
效果：先 ConsumeQi，再 target->TakeDamage(damage)，然后 target->ApplyStatus(effect)
示例：烈焰斩（伤害25 + 灼烧DoT 5点/3回合）
3. 状态效果系统（StatusEffect.h / Character.h）
3.1 状态效果数据结构
cpp
复制
struct StatusEffect {
    std::string name;
    EffectType type;
    int value;
    int duration;
    int maxDuration;
};
3.2 效果类型枚举
cpp
复制
enum class EffectType {
    AttackBuff,    // 攻击提升（RecalculateStats: attackMod += value）
    DefenseBuff,   // 防御提升（RecalculateStats: defenseMod += value）
    AttackDebuff,  // 攻击降低（RecalculateStats: attackMod -= value）
    DefenseDebuff, // 防御降低（RecalculateStats: defenseMod -= value）
    HoT,           // 持续治疗（UpdateEffects: 每轮恢复生命）
    DoT            // 持续伤害（UpdateEffects: 每轮扣除生命）
};
3.3 效果图标字符串（GetTypeString）
AttackBuff -> "攻↑"
DefenseBuff -> "防↑"
AttackDebuff -> "攻↓"
DefenseDebuff -> "防↓"
HoT -> "疗"
DoT -> "毒"
3.4 结算机制
施加时（ApplyStatus）：
同名效果会刷新 duration（不叠加数值）
触发 GameEvent::BuffApplied
调用 RecalculateStats() 重新计算属性修正
每轮结算时（UpdateEffects）：
DoT：直接 attr.hp -= value，可能致死，触发 GameEvent::DotTick
HoT：attr.hp += value（当前未做 maxHp 上限裁剪，存在溢出可能）
所有效果 duration--
过期效果触发 GameEvent::BuffExpired 后移除
最后调用 RecalculateStats()
属性重算（RecalculateStats）：
遍历 activeEffects，累加 attackMod / defenseMod
speedMod 当前恒为 0（没有任何 EffectType 会影响速度）
若属性有变化，触发 GameEvent::StatChanged
3.5 关键限制（当前已知）
DoT/HoT 只在轮次边界结算：TurnManager::StartNewRound() 调用 UpdateEffects()，不是角色行动后立即结算
技能伤害不受 AttackBuff 影响：DamageSkill 和 CompositeSkill 的伤害是构造函数传入的固定值，不读取 GetAttack()
HoT 可能溢出 maxHp：UpdateEffects 里 HoT 直接 attr.hp += value，未做 min(maxHp) 裁剪（Heal 方法有裁剪，但 HoT 走的是直接加法）
速度不受 buff 影响：RecalculateStats 未处理 speedMod，TurnManager 排序时 GetSpeed() 永远返回基础值
同名 buff 不叠加：重复施加同名 StatusEffect 只会刷新 duration
4. 技能库（SkillLibrary.h）
SkillLibrary 是静态工厂类，提供预制的 Action* 实例。所有返回的指针由调用方（Character::AddSkill）接管内存，在 Character 析构时统一 delete。
当前预制技能：
伤害类：Fireball, IceArrow, ThunderStrike, ShadowBolt, Slash, HeavyStrike, PoisonDart
治疗类：Heal, GreatHeal, Regeneration
Buff类：BattleCry, TrueYuanProtect, Fortify, BloodRage
Debuff类：Weaken, Curse
组合类：FlameSlash, VenomStrike, HolySmite
5. 事件类型（GameEvent.h）
与技能/状态相关的事件：
BuffApplied：施加 buff（value 为效果数值，message 含 buff 名）
BuffExpired：buff 到期（message 含 buff 名）
DotTick：DoT/HoT 触发（value 为伤害/治疗数值，message 含描述）
StatChanged：属性因 buff 变化（message 含描述）
QiConsumed / QiRestored：气力变化
Attack / DamageTaken / Healed / Died：常规战斗事件
GameDisplay::Show() 负责将 GameEvent 分发到 BattleUI 的对应静态方法。
6. 玩家回合交互流程（GameController.h）
当前 ProcessPlayerTurn 为同步阻塞的 while 循环结构：
plain
复制
显示状态
while (true):
    主菜单选择（1攻击 2技能 3回蓝）
    if 攻击:
        SelectTarget(Enemy) -> 0返回继续循环 / 执行 PerformAttack -> break
    if 技能:
        SelectSkill(skills) -> 0返回继续循环 / -1返回继续循环
        if 不可用: continue
        根据 skill->GetTargetType() 选择目标队伍
        SelectTarget -> 0返回继续循环(-2) / 执行 skill->Execute -> break
    if 回蓝:
        RestoreQiAction -> break
关键返回值约定：
SelectMainAction：返回枚举 MainActionType
SelectSkill：返回技能索引（0-based），输入 0 返回 -1（表示返回主菜单）
SelectTarget：返回目标索引（0-based），输入 0 返回 -2（表示返回上一层）
7. 如何扩展新技能
7.1 添加新的 EffectType（如 SpeedBuff、Shield、Stun）
在 StatusEffect.h 的 EffectType 枚举中添加新类型
在 StatusEffect::GetTypeString() 中添加对应的显示字符串
在 Character::RecalculateStats() 中处理新类型的数值修正
在 Character::UpdateEffects() 中处理新类型的每回合结算逻辑（如有必要）
在 BattleUI 中添加对应的显示方法（如有特殊显示需求）
7.2 添加新的 Action 派生类
模板：
cpp
复制
class XxxSkill : public Action {
    // 自定义成员
public:
    XxxSkill(const std::string& name, int qiCost, ...)
        : Action(name, qiCost), ... {}
    
    // 可选：重写可用性检查
    bool IsAvailable(const ICombatant* user) const override {
        // 例如：血量低于30%才可用
        return user->GetQi() >= qiCost && user->GetHp() < user->GetMaxHp() * 0.3;
    }
    
    // 可选：重写目标规则
    TargetType GetTargetType() const override { return TargetType::Ally; }
    bool NeedTarget() const override { return true; }  // 或 false
    
    // 必须：实现执行逻辑
    void Execute(ICombatant* user, ICombatant* target) override {
        if (!user || !target) return;
        if (!user->ConsumeQi(qiCost)) return;
        // 自定义效果
        target->ApplyStatus(StatusEffect("xxx", EffectType::Xxx, value, duration));
    }
};
注册到 SkillLibrary：
cpp
复制
static Action* XxxSkill() {
    return new XxxSkill("技能名", 20, ...);
}
注册到角色：
cpp
复制
hero->AddSkill(SkillLibrary::XxxSkill());
7.3 添加带冷却的技能
当前架构没有内置冷却系统。如需实现：
方案 A：在 Action 基类中添加 int cooldown / currentCooldown，在 IsAvailable 中检查，Execute 后设置 currentCooldown = cooldown，在 TurnManager::StartNewRound 中统一减冷却
方案 B：在 Character 中维护冷却映射表
8. 敌人预制（EnemyFactory.h）
EnemyFactory 提供单体敌人和敌人波次群组（Group_Xxx），返回 Character* 或 std::vector<Character*>。所有敌人通过 CreateBase 创建，自动注册 NormalAttack、绑定 GameDisplay::Show 回调。
当前单体敌人：
MiaoRenLouLuo（苗人喽啰）：Slash
MiaoRenTouLing（苗人头领）：HeavyStrike + Fortify
DuZhiZhu（毒蜘蛛）：PoisonDart + VenomStrike
GhostSoldier（鬼兵）：ShadowBolt + Curse
Bandit（山贼）：Slash + Weaken
DarkMage（黑魔法师）：Fireball + ShadowBolt + Curse
PoisonBeast（毒兽）：PoisonDart + Regeneration
当前波次群组：
Group_Novice：2x 喽啰
Group_Elite：头领 + 喽啰
Group_Mixed：毒蜘蛛 + 鬼兵
Group_MageTeam：黑魔法师 + 山贼
Group_Boss：头领 + 2x 喽啰
Group_PoisonNest：毒蜘蛛 + 毒兽
Group_FullAssault：头领 + 黑魔法师 + 山贼
9. 文件依赖关系（技能相关）
plain
复制
SkillLibrary.h
    └─ Action.h
        ├─ ICombatant.h
        │   └─ StatusEffect.h
        ├─ StatusEffect.h
        └─ （各种 Action 派生类）

EnemyFactory.h
    ├─ Character.h / AICharacter.h
    ├─ Action.h
    ├─ SkillLibrary.h
    └─ BattleUI.h（GameDisplay::Show 回调）

Character.h
    ├─ ICombatant.h
    ├─ StatusEffect.h（activeEffects 成员）
    ├─ GameEvent.h（Notify 回调）
    └─ Action.h（前向声明 + skills/actions 列表）

GameController.h
    ├─ BattleUI.h（SelectMainAction / SelectSkill / SelectTarget）
    ├─ BattleManager.h
    ├─ TurnManager.h
    └─ Action.h（RestoreQiAction）

TurnManager.h
    └─ Character.h（UpdateEffects 调用）
10. 已知 Bug / 待改进点
DoT 结算时机：当前在 TurnManager::StartNewRound() 中统一结算，导致毒上完后要等一整轮才跳伤害。如需改为"目标行动前/后结算"，需修改 UpdateEffects 调用位置。
HoT 溢出：UpdateEffects 中 HoT 直接 attr.hp += value，未限制 maxHp。
技能伤害不受攻击 buff 影响：DamageSkill 使用固定值。如需让 buff 影响技能伤害，应改为 target->TakeDamage(user->GetAttack() + bonus) 或引入技能倍率系统。
速度 buff 无效：RecalculateStats 未处理 speedMod，没有任何 EffectType 能影响行动顺序。
AI 随机决策：ProcessEnemyTurn 使用 rand() % 3 随机选择攻击/技能/回蓝，策略极简单。
同名 buff 刷新机制：当前通过 name 字符串匹配判断是否同名，若需支持"不同来源的同名 buff 叠加"，需修改 ApplyStatus 逻辑。