#pragma once
#include <string>

// 升级曲线定义
struct CurveDef {
    float hpFactor = 1.0f;    // 生命成长系数
    float qiFactor = 1.0f;    // 真气成长系数
    float atkFactor = 1.0f;   // 攻击成长系数
    float defFactor = 1.0f;   // 防御成长系数
    float spdFactor = 1.0f;   // 速度成长系数
    int baseExp = 100;        // 1级升2级所需经验
    float expFactor = 1.5f;   // 经验需求倍率
};

// 升级曲线计算器
class GrowthCurve {
public:
    static void RegisterCurve(const std::string& curveId, const CurveDef& def);
    static void ClearCurves();

    static int CalculateHp(const std::string& curveId, int baseHp, int level);
    static int CalculateQi(const std::string& curveId, int baseQi, int level);
    static int CalculateAttack(const std::string& curveId, int baseAttack, int level);
    static int CalculateDefense(const std::string& curveId, int baseDefense, int level);
    static int CalculateSpeed(const std::string& curveId, int baseSpeed, int level);
    static int ExpForNextLevel(const std::string& curveId, int currentLevel);
};
