#include "GrowthCurve.h"
#include <unordered_map>
#include <cmath>

static std::unordered_map<std::string, CurveDef> s_curves;

void GrowthCurve::RegisterCurve(const std::string& curveId, const CurveDef& def) {
    s_curves[curveId] = def;
}

void GrowthCurve::ClearCurves() {
    s_curves.clear();
}

static const CurveDef* GetCurve(const std::string& curveId) {
    auto it = s_curves.find(curveId);
    return (it != s_curves.end()) ? &it->second : nullptr;
}

int GrowthCurve::CalculateHp(const std::string& curveId, int baseHp, int level) {
    auto c = GetCurve(curveId);
    if (!c) return baseHp;
    return static_cast<int>(baseHp * (1.0f + (level - 1) * c->hpFactor * 0.1f));
}

int GrowthCurve::CalculateQi(const std::string& curveId, int baseQi, int level) {
    auto c = GetCurve(curveId);
    if (!c) return baseQi;
    return static_cast<int>(baseQi * (1.0f + (level - 1) * c->qiFactor * 0.1f));
}

int GrowthCurve::CalculateAttack(const std::string& curveId, int baseAttack, int level) {
    auto c = GetCurve(curveId);
    if (!c) return baseAttack;
    return static_cast<int>(baseAttack * (1.0f + (level - 1) * c->atkFactor * 0.1f));
}

int GrowthCurve::CalculateDefense(const std::string& curveId, int baseDefense, int level) {
    auto c = GetCurve(curveId);
    if (!c) return baseDefense;
    return static_cast<int>(baseDefense * (1.0f + (level - 1) * c->defFactor * 0.1f));
}

int GrowthCurve::CalculateSpeed(const std::string& curveId, int baseSpeed, int level) {
    auto c = GetCurve(curveId);
    if (!c) return baseSpeed;
    return static_cast<int>(baseSpeed * (1.0f + (level - 1) * c->spdFactor * 0.1f));
}

int GrowthCurve::ExpForNextLevel(const std::string& curveId, int currentLevel) {
    auto c = GetCurve(curveId);
    if (!c) return 999999;
    return static_cast<int>(c->baseExp * std::pow(c->expFactor, currentLevel - 1));
}
