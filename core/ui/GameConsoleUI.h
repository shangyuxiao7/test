// ==================== GameConsoleUI.h ====================
// 游戏通用控制台 UI 渲染器
// 收拢 main.cpp 中所有非战斗的显示逻辑
#pragma once
#include "../character/PartyManager.h"
#include "../inventory/InventoryManager.h"
#include "../world/WorldFacade.h"
#include "../world/WorldEvent.h"
#include <iostream>
#include <iomanip>
#include <limits>

class GameConsoleUI {
public:
    static void PrintSeparator() {
        std::cout << "========================================\n";
    }

    static void ShowMainMenu() {
        PrintSeparator();
        std::cout << "   PAL1 游戏 Demo\n";
        PrintSeparator();
        std::cout << "1. 开始新游戏\n";
        std::cout << "2. 运行自动测试\n";
        std::cout << "0. 退出\n";
        PrintSeparator();
        std::cout << "请选择: ";
    }

    static void ShowParty(PartyManager& party) {
        std::cout << "\n--- 当前队伍 ---\n";
        int count = party.GetMemberCount();
        if (count == 0) {
            std::cout << "(队伍为空)\n";
        } else {
            for (int i = 0; i < count; ++i) {
                auto* m = party.GetMember(i);
                if (!m) continue;
                std::cout << "  [" << (i + 1) << "] " << m->GetCharacterId()
                          << " Lv." << m->GetLevel()
                          << "  HP:" << m->GetCurrentHp() << "/" << m->GetMaxHp()
                          << "  QI:" << m->GetCurrentQi() << "/" << m->GetMaxQi()
                          << "  ATK:" << m->GetAttack() << "  DEF:" << m->GetDefense()
                          << "  SPD:" << m->GetSpeed()
                          << (m->IsActive() ? " [参战]" : " [待命]")
                          << (m->IsDead() ? " [死亡]" : "")
                          << "\n";
            }
        }
        std::cout << "----------------\n";
    }

    static void ShowInventory(const InventoryManager& inv) {
        std::cout << "\n--- 当前背包 ---\n";
        std::cout << "容量: " << inv.GetItemSlotCount() << " / " << inv.GetCapacity() << "\n";
        std::cout << "金钱: " << inv.GetGold() << "\n";
        auto items = inv.GetItems();
        if (items.empty()) {
            std::cout << "(背包为空)\n";
        } else {
            std::cout << "物品列表:\n";
            int idx = 1;
            for (const auto& stack : items) {
                std::string typeStr;
                switch (stack.GetType()) {
                    case ItemType::Consumable: typeStr = "消耗品"; break;
                    case ItemType::Equipment:  typeStr = "装备"; break;
                    case ItemType::Material:   typeStr = "材料"; break;
                    case ItemType::Quest:      typeStr = "任务"; break;
                }
                std::cout << "  [" << idx++ << "] " << stack.GetName()
                          << " (ID: " << stack.itemId << ") x" << stack.count
                          << " [" << typeStr << "]\n";
            }
        }
        std::cout << "----------------\n";
    }

    static void ShowWorldRoom(const WorldFacade& world) {
        auto room = world.GetCurrentRoom();
        std::cout << "当前: [" << world.GetCurrentMapName() << "] " << room.name << "\n";
        std::cout << room.description << "\n";

        auto exits = world.GetAvailableExits();
        if (!exits.empty()) {
            std::cout << "\n出口:\n";
            for (size_t i = 0; i < exits.size(); ++i) {
                std::cout << "  " << (i + 1) << ". " << exits[i].description << " -> " << exits[i].targetRoomName
                          << (exits[i].isAvailable ? "" : " [未解锁]") << "\n";
            }
        }

        auto entities = world.GetRoomEntities();
        if (!entities.empty()) {
            std::cout << "\n可交互:\n";
            for (size_t i = 0; i < entities.size(); ++i) {
                std::cout << "  " << (i + 1) << ". [" << entities[i].typeName << "] " << entities[i].name
                          << (entities[i].isInteractable ? "" : " [不可交互]") << "\n";
            }
        }
    }

    static void ShowPartyMenu() {
        PrintSeparator();
        std::cout << "队伍管理菜单\n";
        std::cout << "1. 查看队伍\n";
        std::cout << "2. 切换参战/待命\n";
        std::cout << "3. 装备武器/防具\n";
        std::cout << "4. 卸下装备\n";
        std::cout << "0. 返回\n";
        PrintSeparator();
        std::cout << "请选择: ";
    }

    static void ShowInventoryMenu() {
        PrintSeparator();
        std::cout << "背包菜单\n";
        std::cout << "1. 查看背包\n";
        std::cout << "2. 使用物品\n";
        std::cout << "3. 丢弃物品\n";
        std::cout << "0. 返回\n";
        PrintSeparator();
        std::cout << "请选择: ";
    }

    static void ShowWorldEvent(const WorldEvent& e) {
        switch (e.type) {
            case WorldEventType::RequestDialogue:
                std::cout << "[剧情] " << e.param1 << "\n"; break;
            case WorldEventType::RequestBattle:
                std::cout << "[战斗] " << e.param1 << " (波数:" << e.param2 << ")\n"; break;
            case WorldEventType::RequestLoot:
                std::cout << "[掉落] " << e.param1 << " x" << e.param2 << "\n"; break;
            case WorldEventType::RequestTransferMap:
                std::cout << "[传送] " << e.param1 << "\n"; break;
            case WorldEventType::Notification:
                std::cout << "[通知] " << e.param1 << "\n"; break;
            default: break;
        }
    }

    static void ShowGameMenu() {
        PrintSeparator();
        std::cout << "1. 移动\n";
        std::cout << "2. 交互\n";
        std::cout << "3. 队伍管理\n";
        std::cout << "4. 背包\n";
        std::cout << "0. 返回主菜单\n";
        PrintSeparator();
        std::cout << "请选择: ";
    }
};
