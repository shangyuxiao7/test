// ==================== ConsoleUI.h ====================
#pragma once
#include "GameController.h"
#include "GameEvent.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>

class ConsoleUI {
private:
    inline static std::vector<std::string> eventLogs;

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

    static void ClearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    static void DrawTeamPanel(const std::vector<Character*>& team, bool isEnemy) {
        for (auto* c : team) {
            if (!c->IsAlive()) continue;
            std::cout << (isEnemy ? "  [敌方] " : "  [我方] ") << c->GetName() << "\n";
            std::cout << "    HP: " << c->GetHp() << "/" << c->GetMaxHp();
            if (!isEnemy) std::cout << "  Qi: " << c->GetQi() << "/" << c->GetMaxQi();
            std::cout << "  攻:" << c->GetAttack() << " 防:" << c->GetDefense();
            auto buffs = c->GetActiveEffects();
            if (!buffs.empty()) std::cout << " " << BuildBuffString(buffs);
            std::cout << "\n";
        }
    }

    static void DrawBattleStatus(GameController& game) {
        auto order = game.GetActionOrder();
        std::cout << "\n行动顺序: ";
        for (size_t i = 0; i < order.size() && i < 5; ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << order[i].first << "(" << std::fixed << std::setprecision(1) << order[i].second << ")";
        }
        std::cout << "\n\n";

        std::cout << "【敌方队伍】\n";
        DrawTeamPanel(game.GetEnemyTeam(), true);

        std::cout << "\n【我方队伍】\n";
        DrawTeamPanel(game.GetPlayerTeam(), false);

        std::cout << "\n========================================\n";
    }

    static void DrawLogs() {
        int start = (int)eventLogs.size() > 20 ? (int)eventLogs.size() - 20 : 0;
        for (int i = start; i < (int)eventLogs.size(); ++i) {
            std::cout << eventLogs[i] << "\n";
        }
    }

public:
    // 注册为 Character 的 DisplayCallback
    static void HandleEvent(const GameEvent& e) {
        std::string line;
        switch (e.type) {
        case EventType::TurnStart:
            line = "---------- " + e.characterName + " 的回合 ----------";
            break;
        case EventType::Attack:
            line = ">> " + e.characterName + " 对 " + e.targetName + " 发动攻击！(攻击力:" + std::to_string(e.value) + ")";
            break;
        case EventType::DamageTaken:
            line = "   " + e.characterName + " 受到 " + std::to_string(e.value) +
                " 点伤害，剩余 HP:" + std::to_string(e.currentHp) + "/" + std::to_string(e.maxHp);
            break;
        case EventType::Died:
            line = "!!! " + e.characterName + " 被击败 !!!";
            break;
        case EventType::Healed:
            line = "   " + e.characterName + " 恢复 " + std::to_string(e.value) + " 点生命";
            break;
        case EventType::QiConsumed:
            if (e.value == 0) line = "   [" + e.characterName + "] " + e.message;
            else line = "   " + e.characterName + " 消耗 " + std::to_string(e.value) + " 点气力";
            break;
        case EventType::QiRestored:
            line = "   " + e.characterName + " 恢复 " + std::to_string(e.value) + " 点气力";
            break;
        case EventType::BuffApplied:
            line = ">> " + e.characterName + " 获得 [" + e.message + "] 效果";
            if (e.value != 0) line += " (数值:" + std::to_string(e.value) + ")";
            break;
        case EventType::BuffExpired:
            line = ">> " + e.characterName + " 的 [" + e.message + "] 效果已消失";
            break;
        case EventType::DotTick:
            line = ">> " + e.message + " (" + e.characterName + " 数值:" + std::to_string(e.value) + ")";
            break;
        case EventType::StatChanged:
            line = "   [" + e.characterName + "] " + e.message;
            break;
        default:
            return;
        }
        eventLogs.push_back(line);
    }

    // 主循环每帧调用：根据 GameController 状态渲染和交互
    static void Render(GameController& game) {
        // 1. 取走 GameController 的状态日志
        auto logs = game.FlushLogs();
        for (auto& l : logs) eventLogs.push_back(l);

        // 2. 清屏重绘
        ClearScreen();
        DrawBattleStatus(game);
        std::cout << "\n── 战斗日志 ──\n";
        DrawLogs();

        // 3. 根据阶段决定交互
        auto phase = game.GetPhase();

        if (phase == GameController::Phase::PlayerMainMenu) {
            auto* actor = game.GetCurrentActor();
            if (actor) {
                std::cout << "\n【" << actor->GetName() << " 的回合】\n";
                std::cout << "  当前HP " << actor->GetHp() << "/" << actor->GetMaxHp()
                    << "  Qi " << actor->GetQi() << "/" << actor->GetMaxQi()
                    << "  攻 " << actor->GetAttack() << "  防 " << actor->GetDefense() << "\n";
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
                game.SubmitMainAction(static_cast<MainActionType>(choice));
            }
        }
        else if (phase == GameController::Phase::PlayerSkillMenu) {
            auto* actor = game.GetCurrentActor();
            if (!actor) return;
            auto skills = actor->GetSkills();
            if (skills.empty()) {
                std::cout << "\n当前角色没有技能！\n";
                game.SubmitSkill(-1);
                return;
            }
            std::cout << "\n【技能栏】\n";
            for (size_t i = 0; i < skills.size(); ++i) {
                bool available = skills[i]->IsAvailable(actor);
                std::cout << "  " << (i + 1) << ". " << skills[i]->GetName()
                    << " (消耗:" << skills[i]->GetQiCost() << ")";
                if (!available) std::cout << " [不可用]";
                std::string targetDesc;
                switch (skills[i]->GetTargetType()) {
                case TargetType::Enemy: targetDesc = "[敌方]"; break;
                case TargetType::Ally: targetDesc = "[友方]"; break;
                case TargetType::Self: targetDesc = "[自身]"; break;
                case TargetType::None: targetDesc = "[无目标]"; break;
                }
                std::cout << " " << targetDesc << "\n";
            }
            std::cout << "请输入数字(0返回, 1-" << skills.size() << ")：";
            int choice;
            std::cin >> choice;
            while (choice < 0 || choice > (int)skills.size()) {
                std::cout << "输入无效，请重新输入：";
                std::cin >> choice;
            }
            game.SubmitSkill(choice == 0 ? -1 : choice - 1);
        }
        else if (phase == GameController::Phase::PlayerTargetSelect) {
            auto candidates = game.GetTargetCandidates();
            auto tType = game.GetExpectedTargetType();
            std::string prompt;
            switch (tType) {
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
                        << "  HP:" << candidates[i]->GetHp() << "/" << candidates[i]->GetMaxHp()
                        << "  攻:" << candidates[i]->GetAttack()
                        << "  防:" << candidates[i]->GetDefense();
                    auto buffs = candidates[i]->GetActiveEffects();
                    if (!buffs.empty()) std::cout << " " << BuildBuffString(buffs);
                    std::cout << "\n";
                    aliveIndices.push_back((int)i);
                }
            }
            if (aliveIndices.empty()) {
                std::cout << "没有可选目标！\n";
                game.SubmitTarget(-2);
                return;
            }
            std::cout << "请输入数字(0返回, 1-" << aliveIndices.size() << ")：";
            int choice;
            std::cin >> choice;
            while (choice < 0 || choice > (int)aliveIndices.size()) {
                std::cout << "输入无效，请重新输入：";
                std::cin >> choice;
            }
            if (choice == 0) game.SubmitTarget(-2);
            else game.SubmitTarget(aliveIndices[choice - 1]);
        }
        else if (phase == GameController::Phase::Idle || phase == GameController::Phase::Executing) {
            // 非玩家回合，短暂停顿后继续
            std::cout << "\n[按 Enter 继续...]\n";
            std::cin.get();
        }
    }
};