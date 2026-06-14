// ==================== Attributes.h ====================
#pragma once

// 属性结构体 - 纯数据
struct Attributes {
    int hp, maxHp;      // 生命
    int qi, maxQi;      // 气力
    int speed;          // 速度
    int attack;         // 攻击力
    int defense;        // 防御力

    Attributes() : hp(100), maxHp(100), qi(50), maxQi(50), speed(10), attack(20), defense(5) {}
    Attributes(int hp, int qi, int speed, int attack, int defense)
        : hp(hp), maxHp(hp), qi(qi), maxQi(qi), speed(speed), attack(attack), defense(defense) {}
};