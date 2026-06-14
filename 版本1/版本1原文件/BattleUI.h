// ==================== BattleUI.h ====================
#pragma once

#define NOMINMAX

#include "BattleManager.h"
#include "AICharacter.h"
#include "PlayerCharacter.h"
#include "Action.h"
#include "TurnManager.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <thread>
#include <chrono>
#endif

class BattleUI {
private:
    // 计算字符串显示宽度（中文2，英文1）
    static int GetDisplayWidth(const std::string& str) {
        int width = 0;
        for (size_t i = 0; i < str.size(); ) {
            unsigned char c = static_cast<unsigned char>(str[i]);
            if (c >= 0xE0 && c <= 0xEF) {
                width += 2;
                i += 3;
            }
            else if ((c & 0x80) == 0) {
                width += 1;
                ++i;
            }
            else {
                width += 1;
                ++i;
            }
        }
        return width;
    }

    // 居中填充
    static std::string CenterAlign(const std::string& str, int targetWidth) {
        int curWidth = GetDisplayWidth(str);
        if (curWidth >= targetWidth) return str;
        int leftPad = (targetWidth - curWidth) / 2;
        int rightPad = targetWidth - curWidth - leftPad;
        return std::string(leftPad, ' ') + str + std::string(rightPad, ' ');
    }

    // 生成行动顺序字符串（横排，箭头连接）
    static std::string BuildTurnOrderString(const std::vector<Character*>& turnQueue, int currentIndex, int maxShow = 5) {
        if (turnQueue.empty()) return "（无角色）";

        std::vector<std::string> names;
        int queueSize = static_cast<int>(turnQueue.size());
        int idx = currentIndex % queueSize;
        int count = 0;

        while (count < maxShow) {
            Character* c = turnQueue[idx];
            if (c->IsAlive()) {
                names.push_back(c->GetName());
                ++count;
            }
            idx = (idx + 1) % queueSize;
            if (idx == (currentIndex % queueSize) && count == 0) break; // 全死了
        }

        if (names.empty()) return "（无存活角色）";

        std::stringstream ss;
        for (size_t i = 0; i < names.size(); ++i) {
            if (i > 0) ss << " -> ";
            ss << names[i];
        }
        return ss.str();
    }

    // 生成左侧角色状态的多行文本（敌方上方，我方下方，居中格子）
    static std::vector<std::string> BuildTeamPanel(
        const std::vector<Character*>& playerTeam,
        const std::vector<Character*>& enemyTeam) {

        const int BOX_WIDTH = 20;
        const int GAP_WIDTH = 4;
        std::vector<std::string> lines;

        // 收集存活角色
        std::vector<Character*> aliveEnemies, alivePlayers;
        for (auto* e : enemyTeam) if (e->IsAlive()) aliveEnemies.push_back(e);
        for (auto* p : playerTeam) if (p->IsAlive()) alivePlayers.push_back(p);

        // 敌方标题行
        lines.push_back("【敌方队伍】");

        // 敌方名字行
        std::string enemyNameLine;
        for (size_t i = 0; i < aliveEnemies.size(); ++i) {
            enemyNameLine += CenterAlign(aliveEnemies[i]->GetName(), BOX_WIDTH);
            if (i != aliveEnemies.size() - 1) enemyNameLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(enemyNameLine);

        // 敌方血量行
        std::string enemyHpLine;
        for (size_t i = 0; i < aliveEnemies.size(); ++i) {
            auto* c = aliveEnemies[i];
            std::string hpStr = "HP:" + std::to_string(c->GetHp()) + "/" + std::to_string(c->GetMaxHp());
            enemyHpLine += CenterAlign(hpStr, BOX_WIDTH);
            if (i != aliveEnemies.size() - 1) enemyHpLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(enemyHpLine);

        // 空行分隔
        lines.push_back("");

        // 我方标题行
        lines.push_back("【我方队伍】");

        // 我方名字行
        std::string playerNameLine;
        for (size_t i = 0; i < alivePlayers.size(); ++i) {
            playerNameLine += CenterAlign(alivePlayers[i]->GetName(), BOX_WIDTH);
            if (i != alivePlayers.size() - 1) playerNameLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(playerNameLine);

        // 我方血量行
        std::string playerHpLine;
        for (size_t i = 0; i < alivePlayers.size(); ++i) {
            auto* c = alivePlayers[i];
            std::string hpStr = "HP:" + std::to_string(c->GetHp()) + "/" + std::to_string(c->GetMaxHp());
            playerHpLine += CenterAlign(hpStr, BOX_WIDTH);
            if (i != alivePlayers.size() - 1) playerHpLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(playerHpLine);

        // 我方气力行
        std::string playerQiLine;
        for (size_t i = 0; i < alivePlayers.size(); ++i) {
            auto* c = alivePlayers[i];
            std::string qiStr = "Qi:" + std::to_string(c->GetQi()) + "/" + std::to_string(c->GetMaxQi());
            playerQiLine += CenterAlign(qiStr, BOX_WIDTH);
            if (i != alivePlayers.size() - 1) playerQiLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(playerQiLine);

        return lines;
    }

public:
    // 显示战场状态：行动顺序在上，队伍格子在下方
    static void ShowBattleStatus(
        const std::vector<Character*>& playerTeam,
        const std::vector<Character*>& enemyTeam,
        const std::vector<Character*>& turnQueue,
        int currentIndex) {

        // 1. 行动顺序
        std::cout << "\n行动顺序: " << BuildTurnOrderString(turnQueue, currentIndex) << "\n\n";

        // 2. 队伍面板
        auto panelLines = BuildTeamPanel(playerTeam, enemyTeam);
        for (const auto& line : panelLines) {
            std::cout << line << "\n";
        }
        std::cout << "========================================\n";
    }

    // 战斗开始显示（没有行动顺序）
    static void ShowBattleStart(const std::vector<Character*>& playerTeam,
        const std::vector<Character*>& enemyTeam) {
        std::cout << "\n========================================\n";
        std::cout << "           战斗开始！\n";
        auto panelLines = BuildTeamPanel(playerTeam, enemyTeam);
        for (const auto& line : panelLines) {
            std::cout << line << "\n";
        }
        std::cout << "========================================\n";
    }

    static void ShowTurnStart(const std::string& characterName) {
        std::cout << "\n---------- " << characterName << " 的回合 ----------\n";
    }

    static int SelectAction(const std::vector<Action*>& actions) {
        std::cout << "\n请选择操作：\n";
        for (size_t i = 0; i < actions.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << actions[i]->GetName()
                << " (消耗气力:" << actions[i]->GetQiCost() << ")\n";
        }
        std::cout << "请输入数字(1-" << actions.size() << ")：";

        int choice;
        std::cin >> choice;

        while (choice < 1 || choice > static_cast<int>(actions.size())) {
            std::cout << "输入无效，请重新输入(1-" << actions.size() << ")：";
            std::cin >> choice;
        }
        return choice - 1;
    }

    static int SelectTarget(const std::vector<Character*>& enemies) {
        std::cout << "\n请选择目标：\n";
        std::vector<int> aliveIndices;
        for (size_t i = 0; i < enemies.size(); ++i) {
            if (enemies[i]->IsAlive()) {
                std::cout << "  " << (aliveIndices.size() + 1) << ". "
                    << enemies[i]->GetName()
                    << "  HP:" << enemies[i]->GetHp()
                    << "/" << enemies[i]->GetMaxHp() << "\n";
                aliveIndices.push_back(static_cast<int>(i));
            }
        }

        if (aliveIndices.empty()) {
            std::cout << "没有可选目标！\n";
            return -1;
        }

        std::cout << "请输入数字(1-" << aliveIndices.size() << ")：";
        int choice;
        std::cin >> choice;

        while (choice < 1 || choice > static_cast<int>(aliveIndices.size())) {
            std::cout << "输入无效，请重新输入(1-" << aliveIndices.size() << ")：";
            std::cin >> choice;
        }
        return aliveIndices[choice - 1];
    }

    static void ShowAttack(const std::string& attacker, const std::string& target, int attackValue) {
        std::cout << "\n>> " << attacker << " 对 " << target
            << " 发动攻击！(攻击力:" << attackValue << ")\n";
    }

    static void ShowDamage(const std::string& target, int damage, int currentHp, int maxHp) {
        std::cout << "   " << target << " 受到 " << damage
            << " 点伤害，剩余 HP:" << currentHp << "/" << maxHp << "\n";
    }

    static void ShowDeath(const std::string& characterName) {
        std::cout << "\n!!! " << characterName << " 被击败 !!!\n";
    }

    static void ShowBattleResult(bool playerWon) {
        std::cout << "\n========================================\n";
        std::cout << (playerWon ? "         战斗胜利！\n" : "         战斗失败...\n");
        std::cout << "========================================\n";
    }

    static void Pause() {
#ifdef _WIN32
        Sleep(1000);
#else
        std::this_thread::sleep_for(std::chrono::seconds(1));
#endif
    }
};

class GameDisplay {
public:
    static void Show(const GameEvent& e) {
        switch (e.type) {
        case EventType::TurnStart:
            BattleUI::ShowTurnStart(e.characterName);
            break;
        case EventType::Attack:
            BattleUI::ShowAttack(e.characterName, e.targetName, e.value);
            break;
        case EventType::DamageTaken:
            BattleUI::ShowDamage(e.characterName, e.value, e.currentHp, e.maxHp);
            break;
        case EventType::Died:
            BattleUI::ShowDeath(e.characterName);
            break;
        case EventType::BattleWin:
            BattleUI::ShowBattleResult(true);
            break;
        case EventType::BattleLose:
            BattleUI::ShowBattleResult(false);
            break;
        default:
            break;
        }
    }
};