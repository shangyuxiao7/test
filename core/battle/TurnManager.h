// ==================== TurnManager.h ====================
#pragma once
#include "Character.h"
#include <vector>
#include <algorithm>

class TurnManager {
private:
    std::vector<Character*> turnQueue;
    int currentIndex = 0;

public:
    void Initialize(std::vector<Character*>& characters) {
        turnQueue = characters;
        std::sort(turnQueue.begin(), turnQueue.end(),
            [](Character* a, Character* b) {
                return a->GetSpeed() > b->GetSpeed();
            });
        currentIndex = 0;
    }

    Character* GetNextActor() {
        int size = static_cast<int>(turnQueue.size());
        while (currentIndex < size) {
            auto* actor = turnQueue[currentIndex++];
            if (actor->IsAlive()) return actor;
        }
        currentIndex = 0;
        while (currentIndex < size) {
            auto* actor = turnQueue[currentIndex++];
            if (actor->IsAlive()) return actor;
        }
        return nullptr;
    }

    void ResetRound() {
        for (auto* c : turnQueue) {
            if (c->IsAlive()) {
                c->UpdateEffects();
            }
        }
        std::sort(turnQueue.begin(), turnQueue.end(),
            [](Character* a, Character* b) {
                return a->GetSpeed() > b->GetSpeed();
            });
        currentIndex = 0;
    }

    const std::vector<Character*>& GetQueue() const { return turnQueue; }
    int GetCurrentIndex() const { return currentIndex; }
};
