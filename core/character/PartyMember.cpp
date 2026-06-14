#include "PartyMember.h"
#include "CharacterDataTables.h"
#include "GrowthCurve.h"
#include <sstream>

// 辅助解析函数（匿名命名空间，仅限本文件使用）
namespace {
    int ParseIntField(const std::string& json, const std::string& key) {
        size_t pos = json.find(key);
        if (pos == std::string::npos) return 0;
        size_t colon = json.find(":", pos);
        if (colon == std::string::npos) return 0;
        size_t start = colon + 1;
        while (start < json.size() && (json[start] == ' ' || json[start] == '\t')) ++start;
        size_t end = start;
        while (end < json.size() && (json[end] == '-' || (json[end] >= '0' && json[end] <= '9'))) ++end;
        if (start == end) return 0;
        return std::stoi(json.substr(start, end - start));
    }

    std::string ParseStringField(const std::string& json, const std::string& key) {
        size_t pos = json.find(key);
        if (pos == std::string::npos) return "";
        size_t quote = json.find("\"", pos + key.length());
        if (quote == std::string::npos) return "";
        size_t start = quote + 1;
        size_t end = json.find("\"", start);
        if (end == std::string::npos) return "";
        return json.substr(start, end - start);
    }

    std::vector<std::string> ParseStringArray(const std::string& json, const std::string& key) {
        std::vector<std::string> result;
        size_t pos = json.find(key);
        if (pos == std::string::npos) return result;
        size_t bracket = json.find("[", pos);
        if (bracket == std::string::npos) return result;
        size_t close = json.find("]", bracket);
        if (close == std::string::npos) return result;
        std::string content = json.substr(bracket + 1, close - bracket - 1);
        size_t start = 0;
        while (true) {
            size_t q1 = content.find("\"", start);
            if (q1 == std::string::npos) break;
            size_t q2 = content.find("\"", q1 + 1);
            if (q2 == std::string::npos) break;
            result.push_back(content.substr(q1 + 1, q2 - q1 - 1));
            start = q2 + 1;
        }
        return result;
    }
}

PartyMember::PartyMember(const std::string& characterId, int level)
    : characterId(characterId), level(level), currentExp(0), active(true), dead(false) {
    auto def = CharacterDataTables::Instance().GetCharacterDef(characterId);
    if (def) {
        skillIds = def->initialSkills;
    }
    RecalculateStats();
    currentHp = GetBaseMaxHp();
    currentQi = GetBaseMaxQi();
}

void PartyMember::RecalculateStats() {
    // 升级或反序列化后，确保当前HP/QI不超过新的上限
    int newMaxHp = GetBaseMaxHp();
    int newMaxQi = GetBaseMaxQi();
    if (currentHp > newMaxHp) currentHp = newMaxHp;
    if (currentQi > newMaxQi) currentQi = newMaxQi;
}

void PartyMember::GainExp(int amount) {
    if (amount <= 0) return;
    currentExp += amount;
    int needed = GetExpToNextLevel();
    while (currentExp >= needed && needed > 0) {
        currentExp -= needed;
        level++;
        // 升级时恢复20%最大HP和QI作为奖励
        RecalculateStats();
        int healHp = static_cast<int>(GetMaxHp() * 0.2f);
        int healQi = static_cast<int>(GetMaxQi() * 0.2f);
        currentHp = std::min(currentHp + healHp, GetMaxHp());
        currentQi = std::min(currentQi + healQi, GetMaxQi());
        needed = GetExpToNextLevel();
    }
}

void PartyMember::Equip(EquipmentSlot slot, const std::string& equipmentId) {
    if (equipmentId.empty()) {
        Unequip(slot);
        return;
    }
    auto def = CharacterDataTables::Instance().GetEquipmentDef(equipmentId);
    if (def && def->slot == slot) {
        equipments[slot] = equipmentId;
    }
}

void PartyMember::Unequip(EquipmentSlot slot) {
    equipments.erase(slot);
}

void PartyMember::SetActive(bool activeFlag) {
    active = activeFlag;
}

void PartyMember::SetDead(bool deadFlag) {
    dead = deadFlag;
}

void PartyMember::LearnSkill(const std::string& skillId) {
    for (const auto& id : skillIds) {
        if (id == skillId) return; // 已学会，不重复添加
    }
    skillIds.push_back(skillId);
}

CombatantInitData PartyMember::ExportForBattle() const {
    CombatantInitData data;
    data.characterId = characterId;
    auto def = CharacterDataTables::Instance().GetCharacterDef(characterId);
    data.name = def ? def->name : characterId;
    data.level = level;
    data.hp = currentHp;
    data.maxHp = GetMaxHp();
    data.qi = currentQi;
    data.maxQi = GetMaxQi();
    data.speed = GetSpeed();
    data.attack = GetAttack();
    data.defense = GetDefense();
    data.skillIds = skillIds;
    return data;
}

void PartyMember::SyncAfterBattle(int remainingHp, int remainingQi, bool isDead) {
    currentHp = remainingHp;
    currentQi = remainingQi;
    dead = isDead;
    if (currentHp < 0) currentHp = 0;
    if (currentQi < 0) currentQi = 0;
    if (currentHp > GetMaxHp()) currentHp = GetMaxHp();
    if (currentQi > GetMaxQi()) currentQi = GetMaxQi();
}

int PartyMember::GetLevel() const { return level; }
int PartyMember::GetCurrentHp() const { return currentHp; }
int PartyMember::GetMaxHp() const { return GetBaseMaxHp() + GetEquipmentBonusHp(); }
int PartyMember::GetCurrentQi() const { return currentQi; }
int PartyMember::GetMaxQi() const { return GetBaseMaxQi() + GetEquipmentBonusQi(); }

int PartyMember::GetAttack() const { return GetBaseAttack() + GetEquipmentBonusAttack(); }
int PartyMember::GetDefense() const { return GetBaseDefense() + GetEquipmentBonusDefense(); }
int PartyMember::GetSpeed() const { return GetBaseSpeed() + GetEquipmentBonusSpeed(); }

bool PartyMember::IsActive() const { return active; }
bool PartyMember::IsDead() const { return dead; }

std::vector<std::string> PartyMember::GetSkillIds() const { return skillIds; }

std::string PartyMember::GetEquippedItem(EquipmentSlot slot) const {
    auto it = equipments.find(slot);
    return (it != equipments.end()) ? it->second : "";
}

std::string PartyMember::GetCharacterId() const { return characterId; }
int PartyMember::GetCurrentExp() const { return currentExp; }
int PartyMember::GetExpToNextLevel() const {
    auto def = CharacterDataTables::Instance().GetCharacterDef(characterId);
    if (!def) return 999999;
    return GrowthCurve::ExpForNextLevel(def->growthCurveId, level);
}

int PartyMember::GetBaseMaxHp() const {
    auto def = CharacterDataTables::Instance().GetCharacterDef(characterId);
    if (!def) return 0;
    return GrowthCurve::CalculateHp(def->growthCurveId, def->baseHp, level);
}

int PartyMember::GetBaseMaxQi() const {
    auto def = CharacterDataTables::Instance().GetCharacterDef(characterId);
    if (!def) return 0;
    return GrowthCurve::CalculateQi(def->growthCurveId, def->baseQi, level);
}

int PartyMember::GetBaseAttack() const {
    auto def = CharacterDataTables::Instance().GetCharacterDef(characterId);
    if (!def) return 0;
    return GrowthCurve::CalculateAttack(def->growthCurveId, def->baseAttack, level);
}

int PartyMember::GetBaseDefense() const {
    auto def = CharacterDataTables::Instance().GetCharacterDef(characterId);
    if (!def) return 0;
    return GrowthCurve::CalculateDefense(def->growthCurveId, def->baseDefense, level);
}

int PartyMember::GetBaseSpeed() const {
    auto def = CharacterDataTables::Instance().GetCharacterDef(characterId);
    if (!def) return 0;
    return GrowthCurve::CalculateSpeed(def->growthCurveId, def->baseSpeed, level);
}

int PartyMember::GetEquipmentBonusHp() const {
    int bonus = 0;
    for (const auto& kv : equipments) {
        auto def = CharacterDataTables::Instance().GetEquipmentDef(kv.second);
        if (def) bonus += def->bonusHp;
    }
    return bonus;
}

int PartyMember::GetEquipmentBonusQi() const {
    int bonus = 0;
    for (const auto& kv : equipments) {
        auto def = CharacterDataTables::Instance().GetEquipmentDef(kv.second);
        if (def) bonus += def->bonusQi;
    }
    return bonus;
}

int PartyMember::GetEquipmentBonusAttack() const {
    int bonus = 0;
    for (const auto& kv : equipments) {
        auto def = CharacterDataTables::Instance().GetEquipmentDef(kv.second);
        if (def) bonus += def->bonusAttack;
    }
    return bonus;
}

int PartyMember::GetEquipmentBonusDefense() const {
    int bonus = 0;
    for (const auto& kv : equipments) {
        auto def = CharacterDataTables::Instance().GetEquipmentDef(kv.second);
        if (def) bonus += def->bonusDefense;
    }
    return bonus;
}

int PartyMember::GetEquipmentBonusSpeed() const {
    int bonus = 0;
    for (const auto& kv : equipments) {
        auto def = CharacterDataTables::Instance().GetEquipmentDef(kv.second);
        if (def) bonus += def->bonusSpeed;
    }
    return bonus;
}

void PartyMember::Serialize(std::string& out) const {
    std::stringstream ss;
    ss << "{";
    ss << "\"cid\":\"" << characterId << "\",";
    ss << "\"lv\":" << level << ",";
    ss << "\"hp\":" << currentHp << ",";
    ss << "\"qi\":" << currentQi << ",";
    ss << "\"exp\":" << currentExp << ",";
    ss << "\"active\":" << (active ? 1 : 0) << ",";
    ss << "\"dead\":" << (dead ? 1 : 0) << ",";
    ss << "\"skills\":[";
    for (size_t i = 0; i < skillIds.size(); ++i) {
        if (i > 0) ss << ",";
        ss << "\"" << skillIds[i] << "\"";
    }
    ss << "],";
    ss << "\"equips\":[";
    bool first = true;
    for (const auto& kv : equipments) {
        if (!first) ss << ",";
        ss << "{\"slot\":" << static_cast<int>(kv.first) << ",\"id\":\"" << kv.second << "\"}";
        first = false;
    }
    ss << "]}";
    out = ss.str();
}

void PartyMember::Deserialize(const std::string& in) {
    // 解析基本字段
    std::string cid = ParseStringField(in, "\"cid\"");
    if (!cid.empty()) characterId = cid;

    int lv = ParseIntField(in, "\"lv\"");
    if (lv > 0) level = lv;

    currentHp = ParseIntField(in, "\"hp\"");
    currentQi = ParseIntField(in, "\"qi\"");
    currentExp = ParseIntField(in, "\"exp\"");
    active = ParseIntField(in, "\"active\"") != 0;
    dead = ParseIntField(in, "\"dead\"") != 0;
    skillIds = ParseStringArray(in, "\"skills\"");

    // 解析装备
    equipments.clear();
    size_t eqPos = in.find("\"equips\":[");
    if (eqPos != std::string::npos) {
        size_t pos = eqPos + 10;
        while (pos < in.size()) {
            size_t slotPos = in.find("\"slot\":", pos);
            if (slotPos == std::string::npos) break;
            size_t slotStart = slotPos + 7;
            size_t slotEnd = in.find(",", slotStart);
            if (slotEnd == std::string::npos) {
                slotEnd = in.find("}", slotStart);
            }
            int slotVal = std::stoi(in.substr(slotStart, slotEnd - slotStart));

            size_t idPos = in.find("\"id\":\"", slotEnd);
            if (idPos == std::string::npos) break;
            size_t idStart = idPos + 6;
            size_t idEnd = in.find("\"", idStart);
            std::string eqId = in.substr(idStart, idEnd - idStart);

            equipments[static_cast<EquipmentSlot>(slotVal)] = eqId;
            pos = idEnd + 1;
            size_t nextBrace = in.find("}", pos);
            if (nextBrace == std::string::npos) break;
            pos = nextBrace + 1;
        }
    }

    RecalculateStats();
}
