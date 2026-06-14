// ==================== Character.cpp ====================
#include "Character.h"
#include "Action.h"

void Character::AddAction(Action* action) {
    actions.push_back(action);
}