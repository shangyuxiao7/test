// ==================== Character.cpp ====================
#include "Character.h"
#include "Action.h"

void Character::AddAction(Action* action) {
    actions.push_back(action);
}

void Character::AddSkill(Action* action) {
    skills.push_back(action);
}

Character::~Character() {
    for (auto* a : actions) delete a;
    for (auto* s : skills) delete s;
}
