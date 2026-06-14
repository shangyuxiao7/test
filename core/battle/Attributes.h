// ==================== Attributes.h ====================
#pragma once

struct Attributes {
    int hp, maxHp;
    int qi, maxQi;
    int speed;
    int attack;
    int defense;

    Attributes() : hp(100), maxHp(100), qi(50), maxQi(50), speed(10), attack(20), defense(5) {}
    Attributes(int hp, int qi, int speed, int attack, int defense)
        : hp(hp), maxHp(hp), qi(qi), maxQi(qi), speed(speed), attack(attack), defense(defense) {}
};
