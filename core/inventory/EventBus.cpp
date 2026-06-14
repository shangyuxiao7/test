#include "EventBus.h"

EventBus& EventBus::Instance() {
    static EventBus instance;
    return instance;
}

void EventBus::Publish(const InventoryGameEvent& e) {
    history.push_back(e);
}
