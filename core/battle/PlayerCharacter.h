// ==================== PlayerCharacter.h ====================
#pragma once
#include "Character.h"

class PlayerCharacter : public Character {
public:
    using Character::Character;

    void TakeTurn() override {
        Notify(BattleEventType::TurnStart);
    }

    Character* SelectTarget(std::vector<Character*>& enemies) override {
        for (auto* enemy : enemies) {
            if (enemy->IsAlive()) return enemy;
        }
        return nullptr;
    }

    Action* SelectAction() override {
        if (!actions.empty()) return actions[0];
        return nullptr;
    }
};
