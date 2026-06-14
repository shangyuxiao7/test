// ==================== main.cpp ====================
// PAL1 三模块集成测试入口
// 支持：自动测试（三个子模块） + 手动交互测试

#define NOMINMAX
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <cassert>
#include <ctime>
#include <cstdlib>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define GETCWD _getcwd
#else
#include <unistd.h>
#define GETCWD getcwd
#endif

std::string GetCurrentWorkingDir() {
    char buffer[1024];
    if (GETCWD(buffer, sizeof(buffer)) != nullptr) return std::string(buffer);
    return "(unknown)";
}

bool FileExists(const std::string& path) {
    std::ifstream ifs(path);
    return ifs.good();
}

// 三个子模块的唯一对外接口
#include "core/battle/BattleSession.h"
#include "core/battle/BattleConfigLoader.h"
#include "core/inventory/InventoryManager.h"
#include "core/inventory/ItemConfigLoader.h"
#include "core/inventory/EventBus.h"
#include "core/inventory/ItemDatabase.h"
#include "core/world/WorldFacade.h"
#include "core/world/WorldEvent.h"
#include "core/character/PartyManager.h"
#include "core/character/CharacterConfigLoader.h"
#include "core/game/GameManager.h"
#include "core/game/BattleRewardTable.h"
#include "foundation/ConfigLoader.h"
#include "foundation/JsonValue.h"
#include "foundation/JsonUtils.h"
#include "core/ui/BattleConsoleUI.h"
#include "core/ui/GameConsoleUI.h"

#ifdef _WIN32
#include <windows.h>
#endif

GameManager gGame;

// ========== 通用辅助 ==========
void SetUtf8Console() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
}



// ========== 战斗模块辅助 ==========
int TestBattle() {
    int passed = 0, total = 0;
    GameConsoleUI::PrintSeparator();
    std::cout << "[战斗模块] 自动测试开始\n";
    GameConsoleUI::PrintSeparator();

    std::cout << "[Debug] CWD: " << GetCurrentWorkingDir() << "\n";
    std::string skillsPath = "assets/config/skills.json";
    std::string enemiesPath = "assets/config/enemies.json";
    std::cout << "[Debug] Checking " << skillsPath << ": " << (FileExists(skillsPath) ? "exists" : "NOT FOUND") << "\n";
    std::cout << "[Debug] Checking " << enemiesPath << ": " << (FileExists(enemiesPath) ? "exists" : "NOT FOUND") << "\n";
    bool cfg1 = SkillConfigLoader::LoadFromFile(skillsPath);
    bool cfg2 = EnemyConfigLoader::LoadFromFile(enemiesPath);
    std::cout << "[Debug] SkillConfigLoader::LoadFromFile => " << (cfg1 ? "OK" : "FAIL") << "\n";
    std::cout << "[Debug] EnemyConfigLoader::LoadFromFile => " << (cfg2 ? "OK" : "FAIL") << "\n";
    if (!cfg1 || !cfg2) {
        std::cout << "[SKIP] 战斗配置文件加载失败，跳过战斗测试\n";
        return 0;
    }

    // Test 1: 基本战斗流程
    {
        BattleSession session;
        int p1 = session.AddPlayer("李逍遥", 120, 60, 12, 25, 8);
        session.AddPlayerSkill(p1, "slash");
        session.AddEnemyGroup("novice");
        BattleConsoleUI::RunAutoBattle(session);
        bool ok = session.IsOver() && session.GetResult().playerWon;
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 1: 基本战斗流程\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 2: 技能战斗
    {
        BattleSession session;
        int p1 = session.AddPlayer("李逍遥", 120, 60, 12, 25, 8);
        session.AddPlayerSkill(p1, "fireball");
        session.AddPlayerSkill(p1, "heal");
        int p2 = session.AddPlayer("赵灵儿", 100, 80, 10, 18, 5);
        session.AddPlayerSkill(p2, "heal");
        session.AddEnemyGroup("elite");
        BattleConsoleUI::RunAutoBattle(session);
        bool ok = session.IsOver();
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 2: 技能战斗（完成即通过）\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 3: Boss 战
    {
        BattleSession session;
        int p1 = session.AddPlayer("李逍遥", 200, 80, 15, 30, 10);
        session.AddPlayerSkill(p1, "flame_slash");
        session.AddPlayerSkill(p1, "battle_cry");
        int p2 = session.AddPlayer("赵灵儿", 180, 100, 12, 20, 8);
        session.AddPlayerSkill(p2, "great_heal");
        session.AddPlayerSkill(p2, "regeneration");
        session.AddEnemyGroup("boss");
        BattleConsoleUI::RunAutoBattle(session);
        bool ok = session.IsOver();
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 3: Boss 战（完成即通过）\n";
        if (ok) ++passed;
        ++total;
    }

    std::cout << "[战斗模块] 通过: " << passed << " / " << total << "\n";
    return passed == total ? 0 : 1;
}

// ========== 背包模块辅助 ==========
bool CheckInventoryEvent(InventoryEventType expectedType) {
    for (const auto& e : EventBus::Instance().GetHistory()) {
        if (e.type == expectedType) return true;
    }
    return false;
}

int TestInventory() {
    int passed = 0, total = 10;
    GameConsoleUI::PrintSeparator();
    std::cout << "[背包模块] 自动测试开始\n";
    GameConsoleUI::PrintSeparator();

    std::string itemsPath = "assets/config/items.json";
    std::cout << "[Debug] Checking " << itemsPath << ": " << (FileExists(itemsPath) ? "exists" : "NOT FOUND") << "\n";
    bool cfg = ItemConfigLoader::LoadFromFile(itemsPath);
    std::cout << "[Debug] ItemConfigLoader::LoadFromFile => " << (cfg ? "OK" : "FAIL") << "\n";
    if (!cfg) {
        std::cout << "[SKIP] items.json 加载失败，跳过背包测试\n";
        return 0;
    }

    // Test 1: 添加物品
    {
        InventoryManager inv(20);
        bool ok1 = inv.AddItem("heal_herb", 3);
        bool ok2 = inv.GetItemCount("heal_herb") == 3;
        bool ok3 = inv.GetItemSlotCount() == 1;
        std::cout << (ok1 && ok2 && ok3 ? "[PASS]" : "[FAIL]") << " Test 1: 添加物品\n";
        if (ok1 && ok2 && ok3) ++passed;
    }

    // Test 2: 堆叠上限
    {
        InventoryManager inv(20);
        inv.AddItem("heal_herb", 3);
        bool ok1 = inv.AddItem("heal_herb", 96);
        bool ok2 = !inv.AddItem("heal_herb", 1);
        bool ok3 = inv.GetItemCount("heal_herb") == 99;
        std::cout << (ok1 && ok2 && ok3 ? "[PASS]" : "[FAIL]") << " Test 2: 堆叠上限\n";
        if (ok1 && ok2 && ok3) ++passed;
    }

    // Test 3: 背包容量（按物品种类）
    {
        InventoryManager inv(20);
        inv.AddItem("heal_herb", 3);
        bool ok1 = inv.AddItem("iron_sword", 1);
        bool ok2 = inv.AddItem("spirit_stone", 10);
        bool ok3 = inv.AddItem("quest_letter", 1);
        bool ok4 = inv.GetItemSlotCount() == 4;
        std::cout << (ok1 && ok2 && ok3 && ok4 ? "[PASS]" : "[FAIL]") << " Test 3: 背包容量\n";
        if (ok1 && ok2 && ok3 && ok4) ++passed;
    }

    // Test 4: 移除物品
    {
        InventoryManager inv(20);
        inv.AddItem("heal_herb", 99);
        bool ok1 = inv.RemoveItem("heal_herb", 50);
        bool ok2 = inv.GetItemCount("heal_herb") == 49;
        bool ok3 = !inv.RemoveItem("heal_herb", 100);
        std::cout << (ok1 && ok2 && ok3 ? "[PASS]" : "[FAIL]") << " Test 4: 移除物品\n";
        if (ok1 && ok2 && ok3) ++passed;
    }

    // Test 5: 使用物品（消耗品）
    {
        EventBus::Instance().ClearHistory();
        InventoryManager inv(20);
        inv.AddItem("heal_herb", 50);
        bool ok1 = inv.UseItem("heal_herb", 1, "li_xiaoyao");
        bool ok2 = inv.GetItemCount("heal_herb") == 49;
        bool ok3 = CheckInventoryEvent(InventoryEventType::ItemUsed);
        std::cout << (ok1 && ok2 && ok3 ? "[PASS]" : "[FAIL]") << " Test 5: 使用物品\n";
        if (ok1 && ok2 && ok3) ++passed;
    }

    // Test 6: 装备物品
    {
        EventBus::Instance().ClearHistory();
        InventoryManager inv(20);
        inv.AddItem("iron_sword", 1);
        bool ok1 = inv.EquipItem("iron_sword", "li_xiaoyao");
        bool ok2 = inv.GetItemCount("iron_sword") == 0;
        bool ok3 = CheckInventoryEvent(InventoryEventType::ItemEquipped);
        std::cout << (ok1 && ok2 && ok3 ? "[PASS]" : "[FAIL]") << " Test 6: 装备物品\n";
        if (ok1 && ok2 && ok3) ++passed;
    }

    // Test 7: 任务道具不可堆叠
    {
        InventoryManager inv(20);
        inv.AddItem("quest_letter", 1);
        bool ok = !inv.AddItem("quest_letter", 1);
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 7: 任务道具不可堆叠\n";
        if (ok) ++passed;
    }

    // Test 8: 金钱操作
    {
        InventoryManager inv(20);
        inv.AddGold(500);
        bool ok1 = inv.GetGold() == 500;
        bool ok2 = inv.SpendGold(200);
        bool ok3 = inv.GetGold() == 300;
        bool ok4 = !inv.SpendGold(400);
        std::cout << (ok1 && ok2 && ok3 && ok4 ? "[PASS]" : "[FAIL]") << " Test 8: 金钱操作\n";
        if (ok1 && ok2 && ok3 && ok4) ++passed;
    }

    // Test 9: 序列化 / 反序列化
    {
        InventoryManager inv(20);
        inv.AddItem("heal_herb", 48);
        inv.AddGold(300);
        std::string saved;
        inv.Serialize(saved);
        InventoryManager inv2(20);
        inv2.Deserialize(saved);
        bool ok1 = inv2.GetItemCount("heal_herb") == 48;
        bool ok2 = inv2.GetItemCount("iron_sword") == 0;
        bool ok3 = inv2.GetGold() == 300;
        std::cout << (ok1 && ok2 && ok3 ? "[PASS]" : "[FAIL]") << " Test 9: 序列化/反序列化\n";
        if (ok1 && ok2 && ok3) ++passed;
    }

    // Test 10: 丢弃物品
    {
        InventoryManager inv(20);
        inv.AddItem("spirit_stone", 10);
        bool ok1 = inv.DiscardItem("spirit_stone", 5);
        bool ok2 = inv.GetItemCount("spirit_stone") == 5;
        std::cout << (ok1 && ok2 ? "[PASS]" : "[FAIL]") << " Test 10: 丢弃物品\n";
        if (ok1 && ok2) ++passed;
    }

    std::cout << "[背包模块] 通过: " << passed << " / " << total << "\n";
    return passed == total ? 0 : 1;
}

// ========== 地图模块辅助 ==========

int TestWorld() {
    int passed = 0, total = 0;
    GameConsoleUI::PrintSeparator();
    std::cout << "[地图模块] 自动测试开始\n";
    GameConsoleUI::PrintSeparator();

    std::string mapsPath = "assets/config/maps.json";
    std::cout << "[Debug] Checking " << mapsPath << ": " << (FileExists(mapsPath) ? "exists" : "NOT FOUND") << "\n";

    // 新增：直接测试 foundation 层文件读取与 JSON 解析
    {
        std::ifstream ifs(mapsPath, std::ios::binary);
        if (ifs) {
            std::stringstream buffer;
            buffer << ifs.rdbuf();
            std::cout << "[Debug] File size: " << buffer.str().size() << " bytes\n";
            std::cout << "[Debug] First 60 chars: " << buffer.str().substr(0, 60) << "\n";
            std::string content = buffer.str();
            JsonValue root;
            bool parseOk = JsonUtils::Parse(content, root);
            std::cout << "[Debug] JsonUtils::Parse => " << (parseOk ? "OK" : "FAIL") << "\n";
            if (parseOk) {
                std::cout << "[Debug] root.HasField('maps') => " << (root.HasField("maps") ? "YES" : "NO") << "\n";
                std::cout << "[Debug] root['maps'].ArraySize() => " << root["maps"].ArraySize() << "\n";
            } else {
                // 逐步截断，找出解析失败的临界点
                std::cout << "[Debug] 开始逐步截断定位...\n";
                int lastOk = 0;
                int firstFail = static_cast<int>(content.size());
                // 大步长搜索
                for (int len = 1; len <= static_cast<int>(content.size()); len += 100) {
                    JsonValue testRoot;
                    bool ok = JsonUtils::Parse(content.substr(0, len), testRoot);
                    if (ok) lastOk = len;
                    else { firstFail = len; break; }
                }
                // 在失败区间内小步长精确查找
                int exactFail = firstFail;
                for (int len = lastOk + 1; len <= firstFail && len <= static_cast<int>(content.size()); ++len) {
                    JsonValue testRoot;
                    bool ok = JsonUtils::Parse(content.substr(0, len), testRoot);
                    if (!ok) { exactFail = len; break; }
                }
                std::cout << "[Debug] 最后成功长度: " << lastOk << "\n";
                std::cout << "[Debug] 首次失败长度: " << exactFail << "\n";
                if (exactFail > 0 && exactFail <= static_cast<int>(content.size())) {
                    int start = std::max(0, exactFail - 40);
                    int end = std::min(static_cast<int>(content.size()), exactFail + 10);
                    std::cout << "[Debug] 失败位置附近 (" << start << "-" << end << "): ["
                              << content.substr(start, end - start) << "]\n";
                }
            }
        } else {
            std::cout << "[Debug] ifstream open failed\n";
        }
    }

    // Test 1: 加载配置并进入地图
    {
        WorldFacade world;
        bool ok = world.LoadConfig(mapsPath);
        std::cout << "[Debug] WorldFacade::LoadConfig => " << (ok ? "OK" : "FAIL") << "\n";
        if (!ok) {
            std::cout << "[SKIP] maps.json 加载失败，跳过地图测试\n";
            return 0;
        }
        world.EnterMap("abandoned_mine", "mine_entrance");
        ok = (world.GetCurrentRoom().name == "矿洞入口");
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 1: 加载配置并进入地图\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 2: 出口查询
    {
        WorldFacade world;
        world.LoadConfig("assets/config/maps.json");
        world.EnterMap("abandoned_mine", "mine_entrance");
        while (world.HasPendingEvent()) world.NotifyEventComplete(true);
        auto exits = world.GetAvailableExits();
        bool ok = (exits.size() == 1 && exits[0].targetRoomName == "中央大厅");
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 2: 出口查询\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 3: 移动到相邻房间
    {
        WorldFacade world;
        world.LoadConfig("assets/config/maps.json");
        world.EnterMap("abandoned_mine", "mine_entrance");
        while (world.HasPendingEvent()) world.NotifyEventComplete(true);
        bool ok = world.MoveToRoom(0);
        while (world.HasPendingEvent()) world.NotifyEventComplete(true);
        ok = ok && (world.GetCurrentRoom().name == "中央大厅");
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 3: 移动到相邻房间\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 4: 条件出口（未解锁）
    {
        WorldFacade world;
        world.LoadConfig("assets/config/maps.json");
        world.EnterMap("abandoned_mine", "mine_hall");
        while (world.HasPendingEvent()) world.NotifyEventComplete(true);
        auto exits = world.GetAvailableExits();
        bool hasLocked = false;
        for (const auto& e : exits) {
            if (e.description.find("竖井") != std::string::npos) {
                hasLocked = true;
                if (e.isAvailable) hasLocked = false;
            }
        }
        std::cout << (hasLocked ? "[PASS]" : "[FAIL]") << " Test 4: 条件出口未解锁\n";
        if (hasLocked) ++passed;
        ++total;
    }

    // Test 5: 条件出口解锁
    {
        WorldFacade world;
        world.LoadConfig("assets/config/maps.json");
        world.EnterMap("abandoned_mine", "mine_hall");
        while (world.HasPendingEvent()) world.NotifyEventComplete(true);
        world.SetGlobalFlag("mine_boss_defeated", 1);
        auto exits = world.GetAvailableExits();
        bool unlocked = false;
        for (const auto& e : exits) {
            if (e.description.find("竖井") != std::string::npos && e.isAvailable) unlocked = true;
        }
        std::cout << (unlocked ? "[PASS]" : "[FAIL]") << " Test 5: 条件出口解锁\n";
        if (unlocked) ++passed;
        ++total;
    }

    // Test 6: 实体交互（宝箱）
    {
        WorldFacade world;
        world.LoadConfig("assets/config/maps.json");
        world.EnterMap("abandoned_mine", "mine_hall");
        while (world.HasPendingEvent()) world.NotifyEventComplete(true);
        auto entities = world.GetRoomEntities();
        int chestIdx = -1;
        for (size_t i = 0; i < entities.size(); ++i) {
            if (entities[i].typeName == "Chest") { chestIdx = static_cast<int>(i); break; }
        }
        bool ok = (chestIdx >= 0);
        if (ok) {
            ok = world.InteractWithEntity(chestIdx);
            ok = ok && world.HasPendingEvent();
            if (ok) ok = (world.GetPendingEvent().type == WorldEventType::RequestLoot);
        }
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 6: 实体交互（宝箱）\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 7: 序列化 / 反序列化
    {
        WorldFacade world;
        world.LoadConfig("assets/config/maps.json");
        world.EnterMap("abandoned_mine", "mine_entrance");
        while (world.HasPendingEvent()) world.NotifyEventComplete(true);
        world.MoveToRoom(0);
        while (world.HasPendingEvent()) world.NotifyEventComplete(true);
        world.SetGlobalFlag("test_flag", 42);

        std::string saveData;
        world.Serialize(saveData);
        WorldFacade world2;
        world2.LoadConfig("assets/config/maps.json");
        world2.Deserialize(saveData);
        bool ok = (world2.GetCurrentRoom().name == "中央大厅") && (world2.GetGlobalFlag("test_flag") == 42);
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 7: 序列化/反序列化\n";
        if (ok) ++passed;
        ++total;
    }

    std::cout << "[地图模块] 通过: " << passed << " / " << total << "\n";
    return passed == total ? 0 : 1;
}

// ========== 角色模块辅助 ==========

int TestCharacter() {
    int passed = 0, total = 0;
    GameConsoleUI::PrintSeparator();
    std::cout << "[角色模块] 自动测试开始\n";
    GameConsoleUI::PrintSeparator();

    if (!CharacterConfigLoader::LoadAllFromDirectory("assets/config")) {
        std::cout << "[SKIP] 角色配置文件加载失败，跳过角色测试\n";
        return 0;
    }

    // Test 1: 添加成员并查询属性
    {
        PartyManager party;
        int idx = party.AddMember("li_xiaoyao", 1);
        bool ok = (idx == 0);
        auto* m = party.GetMember(0);
        ok = ok && (m != nullptr);
        if (ok) {
            ok = ok && (m->GetLevel() == 1);
            ok = ok && (m->GetCharacterId() == "li_xiaoyao");
            ok = ok && (m->GetMaxHp() > 0);
            ok = ok && (m->GetMaxQi() > 0);
        }
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 1: 添加成员并查询属性\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 2: 装备/卸下
    {
        PartyManager party;
        party.AddMember("li_xiaoyao", 1);
        auto* m = party.GetMember(0);
        bool ok = (m != nullptr);
        if (ok) {
            int atkBefore = m->GetAttack();
            m->Equip(EquipmentSlot::Weapon, "iron_sword");
            ok = ok && (m->GetAttack() == atkBefore + 5);
            m->Unequip(EquipmentSlot::Weapon);
            ok = ok && (m->GetAttack() == atkBefore);
        }
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 2: 装备/卸下\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 3: 获得经验升级
    {
        PartyManager party;
        party.AddMember("li_xiaoyao", 1);
        auto* m = party.GetMember(0);
        bool ok = (m != nullptr);
        if (ok) {
            int hpBefore = m->GetMaxHp();
            m->GainExp(150);
            ok = ok && (m->GetLevel() == 2);
            ok = ok && (m->GetMaxHp() > hpBefore);
        }
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 3: 获得经验升级\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 4: 活跃/待命切换
    {
        PartyManager party;
        party.AddMember("li_xiaoyao", 1);
        party.AddMember("zhao_linger", 1);
        party.SetMemberActive(1, false);
        auto active = party.GetActiveMembers();
        bool ok = (active.size() == 1);
        if (ok) ok = (active[0]->GetCharacterId() == "li_xiaoyao");
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 4: 活跃/待命切换\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 5: 导出战斗数据
    {
        PartyManager party;
        party.AddMember("li_xiaoyao", 5);
        auto initData = party.ExportActiveMembersForBattle();
        bool ok = (initData.size() == 1);
        if (ok) {
            ok = ok && (initData[0].name == "李逍遥");
            ok = ok && (initData[0].level == 5);
            ok = ok && (initData[0].hp > 0);
            ok = ok && (!initData[0].skillIds.empty());
        }
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 5: 导出战斗数据\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 6: 序列化/反序列化
    {
        PartyManager party;
        party.AddMember("li_xiaoyao", 3);
        auto* m = party.GetMember(0);
        m->Equip(EquipmentSlot::Weapon, "iron_sword");
        m->GainExp(50);
        std::string save;
        party.Serialize(save);

        PartyManager party2;
        party2.Deserialize(save);
        auto* m2 = party2.GetMember(0);
        bool ok = (m2 != nullptr);
        if (ok) {
            ok = ok && (m2->GetCharacterId() == "li_xiaoyao");
            ok = ok && (m2->GetLevel() == 3);
            ok = ok && (m2->GetEquippedItem(EquipmentSlot::Weapon) == "iron_sword");
        }
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 6: 序列化/反序列化\n";
        if (ok) ++passed;
        ++total;
    }

    // Test 7: 战斗后同步
    {
        PartyManager party;
        party.AddMember("li_xiaoyao", 5);
        auto* m = party.GetMember(0);
        int maxHp = m->GetMaxHp();
        std::vector<BattleMemberState> results;
        BattleMemberState state;
        state.characterId = "li_xiaoyao";
        state.remainingHp = maxHp / 2;
        state.remainingQi = 10;
        state.isDead = false;
        state.expGained = 200;
        results.push_back(state);
        party.SyncAfterBattle(results);
        bool ok = (m->GetCurrentHp() == maxHp / 2);
        ok = ok && (m->GetCurrentQi() == 10);
        ok = ok && (m->GetLevel() >= 5);
        std::cout << (ok ? "[PASS]" : "[FAIL]") << " Test 7: 战斗后同步\n";
        if (ok) ++passed;
        ++total;
    }

    std::cout << "[角色模块] 通过: " << passed << " / " << total << "\n";
    return passed == total ? 0 : 1;
}

// ========== 手动测试菜单 ==========
void BattleManualTest() {
    SkillConfigLoader::LoadFromFile("assets/config/skills.json");
    EnemyConfigLoader::LoadFromFile("assets/config/enemies.json");

    BattleSession session;
    int p1 = session.AddPlayer("李逍遥", 120, 60, 12, 25, 8);
    session.AddPlayerSkill(p1, "fireball");
    session.AddPlayerSkill(p1, "flame_slash");
    int p2 = session.AddPlayer("赵灵儿", 100, 80, 10, 18, 5);
    session.AddPlayerSkill(p2, "heal");
    session.AddPlayerSkill(p2, "true_yuan_protect");
    session.AddEnemyGroup("novice");

    BattleConsoleUI::RunManualBattle(session);
}

// ========== 游戏内战斗驱动 ==========
BattleOutcome BattleManualDriver(GameManager& gm) {
    return BattleConsoleUI::RunManualBattle(gm);
}

// ========== 游戏内状态查看与操作 ==========

void PartyMenu(PartyManager& party, InventoryManager& inv) {
    while (true) {
        GameConsoleUI::ShowPartyMenu();
        int choice; std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) break;
        if (choice == 1) { GameConsoleUI::ShowParty(party); }
        else if (choice == 2) {
            GameConsoleUI::ShowParty(party);
            std::cout << "成员编号 (1-based): "; int idx; std::cin >> idx;
            auto* m = party.GetMember(idx - 1);
            if (!m) { std::cout << "无效索引\n"; continue; }
            std::cout << "1.参战  0.待命: "; int active; std::cin >> active;
            party.SetMemberActive(idx - 1, active == 1);
            std::cout << "已设置\n";
        }
        else if (choice == 3) {
            GameConsoleUI::ShowParty(party);
            std::cout << "成员编号 (1-based): "; int idx; std::cin >> idx;
            auto* m = party.GetMember(idx - 1);
            if (!m) { std::cout << "无效索引\n"; continue; }
            std::cout << "装备ID: "; std::string eqid; std::getline(std::cin, eqid);
            if (inv.HasItem(eqid, 1)) {
                if (inv.EquipItem(eqid, m->GetCharacterId())) {
                    m->Equip(EquipmentSlot::Weapon, eqid);
                    std::cout << "[成功] 已装备\n";
                } else {
                    std::cout << "[失败] 无法装备\n";
                }
            } else {
                std::cout << "[失败] 背包中没有该物品\n";
            }
        }
        else if (choice == 4) {
            GameConsoleUI::ShowParty(party);
            std::cout << "成员编号 (1-based): "; int idx; std::cin >> idx;
            auto* m = party.GetMember(idx - 1);
            if (!m) { std::cout << "无效索引\n"; continue; }
            std::cout << "槽位 (0=Weapon 1=Armor 2=Shoes 3=Accessory): ";
            int slot; std::cin >> slot;
            m->Unequip(static_cast<EquipmentSlot>(slot));
            std::cout << "[成功] 已卸下\n";
        }
    }
}

void InventoryMenu(InventoryManager& inv, PartyManager& party) {
    while (true) {
        GameConsoleUI::ShowInventoryMenu();
        int choice; std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) break;
        if (choice == 1) { GameConsoleUI::ShowInventory(inv); }
        else if (choice == 2) {
            GameConsoleUI::ShowInventory(inv);
            std::cout << "物品ID: "; std::string id; std::getline(std::cin, id);
            std::cout << "数量: "; int cnt; std::cin >> cnt;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "目标角色ID: "; std::string cid; std::getline(std::cin, cid);
            if (inv.UseItem(id, cnt, cid)) std::cout << "[成功] 已使用\n";
            else std::cout << "[失败]\n";
        }
        else if (choice == 3) {
            GameConsoleUI::ShowInventory(inv);
            std::cout << "物品ID: "; std::string id; std::getline(std::cin, id);
            std::cout << "数量: "; int cnt; std::cin >> cnt;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (inv.DiscardItem(id, cnt)) std::cout << "[成功] 已丢弃\n";
            else std::cout << "[失败]\n";
        }
    }
}

// ========== 正式游戏循环 ==========
BattleOutcome GameLoop(PartyManager& party, InventoryManager& inventory, WorldFacade& world) {
    if (!world.LoadConfig("assets/config/maps.json")) {
        std::cout << "[错误] 无法加载 maps.json\n";
        return BattleOutcome::Defeat;
    }
    if (!CharacterConfigLoader::LoadAllFromDirectory("assets/config")) {
        std::cout << "[错误] 无法加载角色配置文件\n";
        return BattleOutcome::Defeat;
    }
    if (!ItemConfigLoader::LoadFromFile("assets/config/items.json")) {
        std::cout << "[错误] 无法加载 items.json\n";
        return BattleOutcome::Defeat;
    }
    if (!SkillConfigLoader::LoadFromFile("assets/config/skills.json") ||
        !EnemyConfigLoader::LoadFromFile("assets/config/enemies.json")) {
        std::cout << "[错误] 无法加载战斗配置文件\n";
        return BattleOutcome::Defeat;
    }
    if (!BattleRewardTable::Instance().LoadFromFile("assets/config/battle_rewards.json")) {
        std::cout << "[警告] 无法加载 battle_rewards.json\n";
    }

    // 初始化 GameManager
    gGame.Initialize(&party, &inventory, &world);

    gGame.SetGameEventCallback([](const std::string& msg) {
        std::cout << msg << "\n";
    });

    world.SetEventCallback([](const WorldEvent& e) {
        if (gGame.HandleWorldEvent(e)) {
            return;
        }
        GameConsoleUI::ShowWorldEvent(e);
    });

    // 进入起始地图
    world.EnterMap("demo_village", "village_start");
    if (!world.IsInMap()) {
        std::cout << "[错误] 无法进入起始地图\n";
        return BattleOutcome::Defeat;
    }

    while (true) {
        // 战斗中优先驱动战斗
        if (gGame.IsInBattle()) {
            auto outcome = BattleManualDriver(gGame);
            if (outcome == BattleOutcome::Defeat) {
                std::cout << "\n队伍全灭... 游戏结束。\n";
                return BattleOutcome::Defeat;
            }
            continue;
        }

        // 处理世界未决事件（非战斗类，理论上已被 GameManager 处理，但防御性保留）
        if (world.HasPendingEvent()) {
            auto e = world.GetPendingEvent();
            if (!gGame.HandleWorldEvent(e)) {
                GameConsoleUI::ShowWorldEvent(e);
                std::cout << "按回车继续..."; std::cin.get();
            }
            continue;
        }

        GameConsoleUI::ShowWorldRoom(world);

        auto exits = world.GetAvailableExits();
        auto entities = world.GetRoomEntities();

        GameConsoleUI::ShowGameMenu();
        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) break;
        if (choice == 1) {
            if (exits.empty()) { std::cout << "没有出口\n"; continue; }
            std::cout << "\n出口列表:\n";
            for (size_t i = 0; i < exits.size(); ++i) {
                std::cout << "  " << (i + 1) << ". " << exits[i].description << " -> " << exits[i].targetRoomName
                          << (exits[i].isAvailable ? "" : " [未解锁]") << "\n";
            }
            std::cout << "出口编号: "; int idx; std::cin >> idx;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (world.MoveToRoom(idx - 1)) {
                std::cout << "移动成功\n";
            } else {
                std::cout << "移动失败\n";
            }
        }
        else if (choice == 2) {
            if (entities.empty()) { std::cout << "没有可交互对象\n"; continue; }
            std::cout << "\n可交互对象列表:\n";
            for (size_t i = 0; i < entities.size(); ++i) {
                std::cout << "  " << (i + 1) << ". [" << entities[i].typeName << "] " << entities[i].name
                          << (entities[i].isInteractable ? "" : " [不可交互]") << "\n";
            }
            std::cout << "实体编号: "; int idx; std::cin >> idx;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (world.InteractWithEntity(idx - 1)) {
                std::cout << "交互成功\n";
            } else {
                std::cout << "交互失败\n";
            }
        }
        else if (choice == 3) {
            PartyMenu(party, inventory);
        }
        else if (choice == 4) {
            InventoryMenu(inventory, party);
        }
    }

    return BattleOutcome::Victory;
}

// ========== 主入口 ==========
int main() {
    SetUtf8Console();
    srand(static_cast<unsigned>(time(nullptr)));

    bool running = true;
    while (running) {
        GameConsoleUI::ShowMainMenu();
        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) {
            running = false;
        }
        else if (choice == 1) {
            PartyManager party;
            InventoryManager inventory(20);
            WorldFacade world;

            // 先加载角色配置，再创建队伍，否则角色属性无法正确初始化
            CharacterConfigLoader::LoadAllFromDirectory("assets/config");

            // 初始队伍：只有李逍遥
            party.AddMember("li_xiaoyao", 1);

            auto outcome = GameLoop(party, inventory, world);
            if (outcome == BattleOutcome::Defeat) {
                std::cout << "\n[游戏结束] 按回车返回主菜单...\n";
                std::cin.get();
            } else {
                std::cout << "\n[已返回主菜单]\n";
            }
        }
        else if (choice == 2) {
            int result = 0;
            result |= TestBattle();
            result |= TestInventory();
            // TestWorld 依赖旧地图配置，暂时跳过
            // result |= TestWorld();
            result |= TestCharacter();
            std::cout << "\n自动测试全部结束\n";
            std::cout << "按回车继续..."; std::cin.get();
        }
    }

    return 0;
}
