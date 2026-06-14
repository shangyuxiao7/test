// ==================== BattleConsoleUI.h ====================
// 战斗控制台 UI 渲染器
// 参考 pal_battle 的 ConsoleUI.h 设计，适配当前项目接口
#pragma once
#include "../battle/BattleSession.h"
#include "../game/GameManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <limits>

class BattleConsoleUI {
private:
    inline static std::vector<std::string> eventLogs;

    static void OnEvent(const BattleGameEvent& e) {
        std::string line;
        switch (e.type) {
        case BattleEventType::TurnStart:
            line = "---------- " + e.characterName + " 的回合 ----------";
            break;
        case BattleEventType::Attack:
            line = ">> " + e.characterName + " 对 " + e.targetName + " 发动攻击！(攻击力:" + std::to_string(e.value) + ")";
            break;
        case BattleEventType::DamageTaken:
            line = "   " + e.characterName + " 受到 " + std::to_string(e.value) +
                " 点伤害，剩余 HP:" + std::to_string(e.currentHp) + "/" + std::to_string(e.maxHp);
            break;
        case BattleEventType::Died:
            line = "!!! " + e.characterName + " 被击败 !!!";
            break;
        case BattleEventType::Healed:
            line = "   " + e.characterName + " 恢复 " + std::to_string(e.value) + " 点生命";
            break;
        case BattleEventType::QiConsumed:
            if (e.value == 0) line = "   [" + e.characterName + "] " + e.message;
            else line = "   " + e.characterName + " 消耗 " + std::to_string(e.value) + " 点气力";
            break;
        case BattleEventType::QiRestored:
            line = "   " + e.characterName + " 恢复 " + std::to_string(e.value) + " 点气力";
            break;
        case BattleEventType::BuffApplied:
            line = ">> " + e.characterName + " 获得 [" + e.message + "] 效果";
            if (e.value != 0) line += " (数值:" + std::to_string(e.value) + ")";
            break;
        case BattleEventType::BuffExpired:
            line = ">> " + e.characterName + " 的 [" + e.message + "] 效果已消失";
            break;
        case BattleEventType::DotTick:
            line = ">> " + e.message + " (" + e.characterName + " 数值:" + std::to_string(e.value) + ")";
            break;
        case BattleEventType::StatChanged:
            line = "   [" + e.characterName + "] " + e.message;
            break;
        case BattleEventType::BattleStart:
            line = "========================================";
            eventLogs.push_back(line);
            line = "战斗开始!";
            eventLogs.push_back(line);
            line = "========================================";
            break;
        case BattleEventType::BattleWin:
            line = "========================================";
            eventLogs.push_back(line);
            line = "战斗胜利!";
            eventLogs.push_back(line);
            line = "========================================";
            break;
        case BattleEventType::BattleLose:
            line = "========================================";
            eventLogs.push_back(line);
            line = "战斗失败...";
            eventLogs.push_back(line);
            line = "========================================";
            break;
        default:
            return;
        }
        eventLogs.push_back(line);
    }

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

    static void DrawTeamPanel(const std::vector<Character*>& team, bool isEnemy) {
        for (auto* c : team) {
            if (!c->IsAlive()) continue;
            std::cout << (isEnemy ? "  [敌方] " : "  [我方] ") << c->GetName() << "\n";
            std::cout << "    HP: " << c->GetHp() << "/" << c->GetMaxHp();
            if (!isEnemy) std::cout << "  气力: " << c->GetQi() << "/" << c->GetMaxQi();
            std::cout << "  攻:" << c->GetAttack() << "  防:" << c->GetDefense();
            auto buffs = c->GetActiveEffects();
            if (!buffs.empty()) std::cout << " " << BuildBuffString(buffs);
            std::cout << "\n";
        }
    }

    static void DrawBattleStatus(BattleSession& session) {
        auto order = session.GetActionOrder();
        std::cout << "\n行动顺序: ";
        for (size_t i = 0; i < order.size() && i < 5; ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << order[i].first << "(" << std::fixed << std::setprecision(1) << order[i].second << ")";
        }
        std::cout << "\n\n";

        std::cout << "【敌方队伍】\n";
        DrawTeamPanel(session.GetEnemyTeam(), true);

        std::cout << "\n【我方队伍】\n";
        DrawTeamPanel(session.GetPlayerTeam(), false);

        std::cout << "\n========================================\n";
    }

    static void DrawLogs() {
        for (size_t i = 0; i < eventLogs.size(); ++i) {
            std::cout << eventLogs[i] << "\n";
        }
    }

    static bool PromptMainAction(BattleSession& session) {
        auto* actor = session.GetCurrentActor();
        if (!actor) return false;
        std::cout << "\n【" << actor->GetName() << " 的回合】\n";
        std::cout << "  当前 HP " << actor->GetHp() << "/" << actor->GetMaxHp()
            << "  气力 " << actor->GetQi() << "/" << actor->GetMaxQi()
            << "  攻 " << actor->GetAttack() << "  防 " << actor->GetDefense() << "\n";
        std::cout << "  1. 普通攻击\n";
        std::cout << "  2. 技能\n";
        std::cout << "  3. 回气\n";
        std::cout << "  0. 自动运行\n";
        std::cout << "请输入数字: ";
        int choice;
        std::cin >> choice;
        if (choice == 0) {
            RunAutoBattle(session);
            return false;
        }
        while (choice < 1 || choice > 3) {
            std::cout << "输入无效，请重新输入(1-3, 0=自动): ";
            std::cin >> choice;
            if (choice == 0) {
                RunAutoBattle(session);
                return false;
            }
        }
        session.SubmitMainAction(static_cast<MainActionType>(choice));
        return false;
    }

    static void PromptSkillSelection(BattleSession& session) {
        auto* actor = session.GetCurrentActor();
        if (!actor) return;
        auto skills = actor->GetSkills();
        if (skills.empty()) {
            std::cout << "\n当前角色没有技能！\n";
            session.SubmitSkill(-1);
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
        std::cout << "请输入数字(0返回, 1-" << skills.size() << "): ";
        int choice;
        std::cin >> choice;
        while (choice < 0 || choice >(int)skills.size()) {
            std::cout << "输入无效，请重新输入: ";
            std::cin >> choice;
        }
        session.SubmitSkill(choice == 0 ? -1 : choice - 1);
    }

    static void PromptTargetSelection(BattleSession& session) {
        auto candidates = session.GetTargetCandidates();
        auto tType = session.GetExpectedTargetType();
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
            session.SubmitTarget(-2);
            return;
        }
        std::cout << "请输入数字(0返回, 1-" << aliveIndices.size() << "): ";
        int choice;
        std::cin >> choice;
        while (choice < 0 || choice >(int)aliveIndices.size()) {
            std::cout << "输入无效，请重新输入: ";
            std::cin >> choice;
        }
        if (choice == 0) session.SubmitTarget(-2);
        else session.SubmitTarget(aliveIndices[choice - 1]);
    }

public:
    static void Bind(BattleSession& session) {
        session.SetDisplayCallback(OnEvent);
    }

    static void ClearLogs() {
        eventLogs.clear();
    }

    // 自动运行一场战斗（AI 控制玩家全部选攻击）
    static void RunAutoBattle(BattleSession& session) {
        while (!session.IsOver()) {
            session.Update();
            auto phase = session.GetPhase();
            if (phase == GameController::Phase::PlayerMainMenu) {
                session.SubmitMainAction(MainActionType::Attack);
            }
            else if (phase == GameController::Phase::PlayerSkillMenu) {
                session.SubmitSkill(-1);
            }
            else if (phase == GameController::Phase::PlayerTargetSelect) {
                session.SubmitTarget(0);
            }
        }
    }

    // 运行一场手动战斗（直接操作 BattleSession）。
    // 返回 Victory / Defeat。
    static BattleOutcome RunManualBattle(BattleSession& session) {
        ClearLogs();
        Bind(session);
        session.Start();

        while (!session.IsOver()) {
            session.Update();

            // 获取并缓存 session 日志
            auto logs = session.FlushLogs();
            for (auto& l : logs) eventLogs.push_back(l);

            auto phase = session.GetPhase();
            if (phase == GameController::Phase::PlayerMainMenu) {
                if (session.IsOver()) break;
                // 只在玩家回合开始时渲染完整画面（状态面板 + 日志 + 菜单）
                DrawBattleStatus(session);
                std::cout << "\n── 战斗日志 ──\n";
                DrawLogs();
                PromptMainAction(session);
            }
            else if (phase == GameController::Phase::PlayerSkillMenu) {
                // 只显示技能菜单，不重复渲染面板
                PromptSkillSelection(session);
            }
            else if (phase == GameController::Phase::PlayerTargetSelect) {
                // 只显示目标选择，不重复渲染面板
                PromptTargetSelection(session);
            }
            else if (phase == GameController::Phase::BattleOver) {
                break;
            }
            // Idle / Executing / EnemyTurn：继续推进，不渲染，不暂停
        }

        // 战斗结束后最后渲染一次
        auto logs = session.FlushLogs();
        for (auto& l : logs) eventLogs.push_back(l);
        DrawBattleStatus(session);
        std::cout << "\n── 战斗日志 ──\n";
        DrawLogs();

        auto result = session.GetResult();
        return result.playerWon ? BattleOutcome::Victory : BattleOutcome::Defeat;
    }

    // 运行一场手动战斗（通过 GameManager）。
    // 内部通过 GetCurrentBattleSession() 获取 session 指针。
    static BattleOutcome RunManualBattle(GameManager& gm) {
        ClearLogs();

        while (gm.IsInBattle()) {
            auto* session = gm.GetCurrentBattleSession();
            if (!session) return BattleOutcome::Defeat;

            // 确保当前 session 绑定了 UI 回调（多波次时 session 会更换）
            Bind(*session);

            bool finished = gm.BattleTick();

            // 获取并缓存日志
            auto logs = session->FlushLogs();
            for (auto& l : logs) eventLogs.push_back(l);

            if (finished) {
                // 战斗结束，渲染最终画面
                DrawBattleStatus(*session);
                std::cout << "\n── 战斗日志 ──\n";
                DrawLogs();
                auto result = gm.GetBattleResult();
                if (result.playerWon) {
                    std::cout << "\n===== 战斗胜利！=====\n";
                    std::cout << "战后结算中...\n";
                    gm.FinalizeBattle(true);
                    return BattleOutcome::Victory;
                } else {
                    std::cout << "\n===== 战斗失败... =====\n";
                    gm.FinalizeBattle(false);
                    return BattleOutcome::Defeat;
                }
            }

            auto phase = gm.GetBattlePhase();
            if (phase == GameController::Phase::PlayerMainMenu) {
                // 只在玩家回合开始时渲染完整画面（状态面板 + 日志 + 菜单）
                DrawBattleStatus(*session);
                std::cout << "\n── 战斗日志 ──\n";
                DrawLogs();
                PromptMainAction(*session);
            }
            else if (phase == GameController::Phase::PlayerSkillMenu) {
                // 只显示技能菜单，不重复渲染面板
                PromptSkillSelection(*session);
            }
            else if (phase == GameController::Phase::PlayerTargetSelect) {
                // 只显示目标选择，不重复渲染面板
                PromptTargetSelection(*session);
            }
            // Idle / Executing / EnemyTurn：继续推进，不渲染，不暂停
        }

        return BattleOutcome::Defeat;
    }
};
