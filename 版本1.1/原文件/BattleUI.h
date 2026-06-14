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

// 主菜单选项
enum class MainActionType { Attack = 1, Skill, RestoreQi };

class BattleUI {
private:
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

    static std::string CenterAlign(const std::string& str, int targetWidth) {
        int curWidth = GetDisplayWidth(str);
        if (curWidth >= targetWidth) return str;
        int leftPad = (targetWidth - curWidth) / 2;
        int rightPad = targetWidth - curWidth - leftPad;
        return std::string(leftPad, ' ') + str + std::string(rightPad, ' ');
    }

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
            if (idx == (currentIndex % queueSize) && count == 0) break;
        }

        if (names.empty()) return "（无存活角色）";

        std::stringstream ss;
        for (size_t i = 0; i < names.size(); ++i) {
            if (i > 0) ss << " -> ";
            ss << names[i];
        }
        return ss.str();
    }

    // 构建 buff 显示字符串
    static std::string BuildBuffString(const std::vector<StatusEffect>& effects) {
        if (effects.empty()) return "";
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < effects.size(); ++i) {
            if (i > 0) ss << " ";
            ss << effects[i].GetTypeString() << effects[i].duration;
        }
        ss << "]";
        return ss.str();
    }

    static std::vector<std::string> BuildTeamPanel(
        const std::vector<Character*>& playerTeam,
        const std::vector<Character*>& enemyTeam) {

        const int BOX_WIDTH = 22;
        const int GAP_WIDTH = 4;
        std::vector<std::string> lines;

        std::vector<Character*> aliveEnemies, alivePlayers;
        for (auto* e : enemyTeam) if (e->IsAlive()) aliveEnemies.push_back(e);
        for (auto* p : playerTeam) if (p->IsAlive()) alivePlayers.push_back(p);

        // 敌方
        lines.push_back("【敌方队伍】");
        std::string enemyNameLine;
        for (size_t i = 0; i < aliveEnemies.size(); ++i) {
            enemyNameLine += CenterAlign(aliveEnemies[i]->GetName(), BOX_WIDTH);
            if (i != aliveEnemies.size() - 1) enemyNameLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(enemyNameLine);

        std::string enemyHpLine;
        for (size_t i = 0; i < aliveEnemies.size(); ++i) {
            auto* c = aliveEnemies[i];
            std::string hpStr = "HP:" + std::to_string(c->GetHp()) + "/" + std::to_string(c->GetMaxHp());
            enemyHpLine += CenterAlign(hpStr, BOX_WIDTH);
            if (i != aliveEnemies.size() - 1) enemyHpLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(enemyHpLine);

        std::string enemyBuffLine;
        for (size_t i = 0; i < aliveEnemies.size(); ++i) {
            std::string buffStr = BuildBuffString(aliveEnemies[i]->GetActiveEffects());
            if (buffStr.empty()) buffStr = " ";
            enemyBuffLine += CenterAlign(buffStr, BOX_WIDTH);
            if (i != aliveEnemies.size() - 1) enemyBuffLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(enemyBuffLine);

        lines.push_back("");

        // 我方
        lines.push_back("【我方队伍】");
        std::string playerNameLine;
        for (size_t i = 0; i < alivePlayers.size(); ++i) {
            playerNameLine += CenterAlign(alivePlayers[i]->GetName(), BOX_WIDTH);
            if (i != alivePlayers.size() - 1) playerNameLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(playerNameLine);

        std::string playerHpLine;
        for (size_t i = 0; i < alivePlayers.size(); ++i) {
            auto* c = alivePlayers[i];
            std::string hpStr = "HP:" + std::to_string(c->GetHp()) + "/" + std::to_string(c->GetMaxHp());
            playerHpLine += CenterAlign(hpStr, BOX_WIDTH);
            if (i != alivePlayers.size() - 1) playerHpLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(playerHpLine);

        std::string playerQiLine;
        for (size_t i = 0; i < alivePlayers.size(); ++i) {
            auto* c = alivePlayers[i];
            std::string qiStr = "Qi:" + std::to_string(c->GetQi()) + "/" + std::to_string(c->GetMaxQi());
            playerQiLine += CenterAlign(qiStr, BOX_WIDTH);
            if (i != alivePlayers.size() - 1) playerQiLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(playerQiLine);

        std::string playerBuffLine;
        for (size_t i = 0; i < alivePlayers.size(); ++i) {
            std::string buffStr = BuildBuffString(alivePlayers[i]->GetActiveEffects());
            if (buffStr.empty()) buffStr = " ";
            playerBuffLine += CenterAlign(buffStr, BOX_WIDTH);
            if (i != alivePlayers.size() - 1) playerBuffLine += std::string(GAP_WIDTH, ' ');
        }
        lines.push_back(playerBuffLine);

        return lines;
    }

public:
    static void ShowBattleStatus(
        const std::vector<Character*>& playerTeam,
        const std::vector<Character*>& enemyTeam,
        const std::vector<Character*>& turnQueue,
        int currentIndex) {

        std::cout << "\n行动顺序: " << BuildTurnOrderString(turnQueue, currentIndex) << "\n\n";

        auto panelLines = BuildTeamPanel(playerTeam, enemyTeam);
        for (const auto& line : panelLines) {
            std::cout << line << "\n";
        }
        std::cout << "========================================\n";
    }

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

    // 主菜单选择
    static MainActionType SelectMainAction() {
        std::cout << "\n请选择操作：\n";
        std::cout << "  1. 普通攻击\n";
        std::cout << "  2. 技能\n";
        std::cout << "  3. 回蓝\n";
        std::cout << "请输入数字(1-3)：";
        int choice;
        std::cin >> choice;
        while (choice < 1 || choice > 3) {
            std::cout << "输入无效，请重新输入(1-3)：";
            std::cin >> choice;
        }
        return static_cast<MainActionType>(choice);
    }

    // 技能子菜单（展示可用性，支持0返回）
    static int SelectSkill(const std::vector<Action*>& skills, const ICombatant* user) {
        std::cout << "\n【技能栏】\n";
        for (size_t i = 0; i < skills.size(); ++i) {
            bool available = skills[i]->IsAvailable(user);
            std::cout << "  " << (i + 1) << ". " << skills[i]->GetName()
                << " (消耗气力:" << skills[i]->GetQiCost() << ")";
            if (!available) {
                std::cout << " [不可用]";
            }
            std::string targetDesc;
            switch (skills[i]->GetTargetType()) {
            case TargetType::Enemy: targetDesc = "敌方"; break;
            case TargetType::Ally: targetDesc = "友方"; break;
            case TargetType::Self: targetDesc = "自身"; break;
            case TargetType::None: targetDesc = "无目标"; break;
            }
            std::cout << " [" << targetDesc << "]\n";
        }
        std::cout << "请输入数字(0返回, 1-" << skills.size() << ")：";
        int choice;
        std::cin >> choice;

        while (choice < 0 || choice > static_cast<int>(skills.size())) {
            std::cout << "输入无效，请重新输入(0返回, 1-" << skills.size() << ")：";
            std::cin >> choice;
        }
        if (choice == 0) return -1; // 返回主菜单
        return choice - 1;
    }

    // 增强版目标选择（根据 TargetType 切换提示和候选队伍，支持0返回）
    static int SelectTarget(const std::vector<Character*>& candidates, TargetType type) {
        if (type == TargetType::Self) return -1;

        std::string prompt;
        switch (type) {
        case TargetType::Enemy: prompt = "请选择敌方目标："; break;
        case TargetType::Ally: prompt = "请选择友方目标："; break;
        default: prompt = "请选择目标："; break;
        }

        std::cout << "\n" << prompt << "\n";
        std::vector<int> aliveIndices;
        for (size_t i = 0; i < candidates.size(); ++i) {
            if (candidates[i]->IsAlive()) {
                std::cout << "  " << (aliveIndices.size() + 1) << ". "
                    << candidates[i]->GetName()
                    << "  HP:" << candidates[i]->GetHp()
                    << "/" << candidates[i]->GetMaxHp()
                    << "  攻:" << candidates[i]->GetAttack()
                    << "  防:" << candidates[i]->GetDefense();
                auto buffs = candidates[i]->GetActiveEffects();
                if (!buffs.empty()) {
                    std::cout << " " << BuildBuffString(buffs);
                }
                std::cout << "\n";
                aliveIndices.push_back(static_cast<int>(i));
            }
        }

        if (aliveIndices.empty()) {
            std::cout << "没有可选目标！\n";
            return -1;
        }

        std::cout << "请输入数字(0返回, 1-" << aliveIndices.size() << ")：";
        int choice;
        std::cin >> choice;

        while (choice < 0 || choice > static_cast<int>(aliveIndices.size())) {
            std::cout << "输入无效，请重新输入(0返回, 1-" << aliveIndices.size() << ")：";
            std::cin >> choice;
        }
        if (choice == 0) return -2; // 返回上一层（主菜单）
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

    static void ShowBuffApplied(const std::string& characterName, const std::string& buffName, int value) {
        std::cout << "\n>> " << characterName << " 获得 [" << buffName << "] 效果";
        if (value != 0) std::cout << " (数值:" << value << ")";
        std::cout << "\n";
    }

    static void ShowBuffExpired(const std::string& characterName, const std::string& buffName) {
        std::cout << "\n>> " << characterName << " 的 [" << buffName << "] 效果已消失\n";
    }

    static void ShowDotTick(const std::string& characterName, int value, const std::string& msg) {
        std::cout << "\n>> " << msg << " (" << characterName << " 数值:" << value << ")\n";
    }

    static void ShowStatChanged(const std::string& characterName, const std::string& msg) {
        std::cout << "   [" << characterName << "] " << msg << "\n";
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
        case EventType::BuffApplied:
            BattleUI::ShowBuffApplied(e.characterName, e.message, e.value);
            break;
        case EventType::BuffExpired:
            BattleUI::ShowBuffExpired(e.characterName, e.message);
            break;
        case EventType::DotTick:
            BattleUI::ShowDotTick(e.characterName, e.value, e.message);
            break;
        case EventType::StatChanged:
            BattleUI::ShowStatChanged(e.characterName, e.message);
            break;
        default:
            break;
        }
    }
};