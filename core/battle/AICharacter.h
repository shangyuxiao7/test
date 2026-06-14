// ==================== AICharacter.h ====================
#pragma once
#include "Character.h"
#include <cstdlib>

class AICharacter : public Character {
public:
    using Character::Character;

    void TakeTurn() override {
        Notify(BattleEventType::TurnStart);
    }

    Character* SelectTarget(std::vector<Character*>& enemies) override {
        std::vector<Character*> aliveTargets;
        for (auto* target : enemies) {
            if (target->IsAlive()) {
                aliveTargets.push_back(target);
            }
        }
        if (aliveTargets.empty()) return nullptr;
        int randomIndex = rand() % aliveTargets.size();
        return aliveTargets[randomIndex];
    }

    Action* SelectAction() override {
        if (!actions.empty()) return actions[0];
        return nullptr;
    }
};
