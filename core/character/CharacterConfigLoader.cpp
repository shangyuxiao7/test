#include "CharacterConfigLoader.h"
#include "CharacterDataTables.h"
#include "GrowthCurve.h"
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

// 简易 JSON 解析辅助（匿名命名空间）
namespace {
    std::string ReadFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return "";
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        // 跳过 UTF-8 BOM
        if (content.size() >= 3 &&
            static_cast<unsigned char>(content[0]) == 0xEF &&
            static_cast<unsigned char>(content[1]) == 0xBB &&
            static_cast<unsigned char>(content[2]) == 0xBF) {
            content = content.substr(3);
        }
        return content;
    }

    std::string ExtractArray(const std::string& json, const std::string& key) {
        size_t pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        pos = json.find("[", pos);
        if (pos == std::string::npos) return "";
        size_t end = pos + 1;
        int depth = 1;
        while (end < json.size() && depth > 0) {
            if (json[end] == '[') ++depth;
            else if (json[end] == ']') --depth;
            ++end;
        }
        if (depth != 0) return "";
        return json.substr(pos + 1, end - pos - 2);
    }

    std::vector<std::string> ExtractObjects(const std::string& arrayContent) {
        std::vector<std::string> result;
        size_t pos = 0;
        while (pos < arrayContent.size()) {
            size_t start = arrayContent.find("{", pos);
            if (start == std::string::npos) break;
            size_t end = start + 1;
            int depth = 1;
            while (end < arrayContent.size() && depth > 0) {
                if (arrayContent[end] == '{') ++depth;
                else if (arrayContent[end] == '}') --depth;
                ++end;
            }
            if (depth != 0) break;
            result.push_back(arrayContent.substr(start, end - start));
            pos = end;
        }
        return result;
    }

    std::string ParseString(const std::string& obj, const std::string& key) {
        size_t pos = obj.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        pos = obj.find("\"", pos + key.length() + 2);
        if (pos == std::string::npos) return "";
        size_t end = obj.find("\"", pos + 1);
        if (end == std::string::npos) return "";
        return obj.substr(pos + 1, end - pos - 1);
    }

    int ParseInt(const std::string& obj, const std::string& key, int defaultVal = 0) {
        size_t pos = obj.find("\"" + key + "\"");
        if (pos == std::string::npos) return defaultVal;
        pos = obj.find(":", pos);
        if (pos == std::string::npos) return defaultVal;
        ++pos;
        while (pos < obj.size() && (obj[pos] == ' ' || obj[pos] == '\t' || obj[pos] == '\n' || obj[pos] == '\r')) ++pos;
        size_t end = pos;
        while (end < obj.size() && (obj[end] == '-' || obj[end] == '.' || (obj[end] >= '0' && obj[end] <= '9'))) ++end;
        if (pos == end) return defaultVal;
        try { return std::stoi(obj.substr(pos, end - pos)); } catch (...) { return defaultVal; }
    }

    float ParseFloat(const std::string& obj, const std::string& key, float defaultVal = 0.0f) {
        size_t pos = obj.find("\"" + key + "\"");
        if (pos == std::string::npos) return defaultVal;
        pos = obj.find(":", pos);
        if (pos == std::string::npos) return defaultVal;
        ++pos;
        while (pos < obj.size() && (obj[pos] == ' ' || obj[pos] == '\t' || obj[pos] == '\n' || obj[pos] == '\r')) ++pos;
        size_t end = pos;
        while (end < obj.size() && (obj[end] == '-' || obj[end] == '.' || (obj[end] >= '0' && obj[end] <= '9'))) ++end;
        if (pos == end) return defaultVal;
        try { return std::stof(obj.substr(pos, end - pos)); } catch (...) { return defaultVal; }
    }

    std::vector<std::string> ParseStringArray(const std::string& obj, const std::string& key) {
        std::vector<std::string> result;
        size_t pos = obj.find("\"" + key + "\"");
        if (pos == std::string::npos) return result;
        pos = obj.find("[", pos);
        if (pos == std::string::npos) return result;
        size_t end = pos + 1;
        int depth = 1;
        while (end < obj.size() && depth > 0) {
            if (obj[end] == '[') ++depth;
            else if (obj[end] == ']') --depth;
            ++end;
        }
        if (depth != 0) return result;
        std::string content = obj.substr(pos + 1, end - pos - 2);
        size_t q = 0;
        while (true) {
            q = content.find("\"", q);
            if (q == std::string::npos) break;
            size_t q2 = content.find("\"", q + 1);
            if (q2 == std::string::npos) break;
            result.push_back(content.substr(q + 1, q2 - q - 1));
            q = q2 + 1;
        }
        return result;
    }
}

bool CharacterConfigLoader::LoadCharactersFromFile(const std::string& filepath) {
    std::string json = ReadFile(filepath);
    if (json.empty()) return false;

    std::string arrayContent = ExtractArray(json, "characters");
    auto objects = ExtractObjects(arrayContent);

    CharacterDataTables::Instance().ClearCharacters();

    for (const auto& obj : objects) {
        CharacterDef def;
        def.id = ParseString(obj, "id");
        def.name = ParseString(obj, "name");
        def.baseHp = ParseInt(obj, "baseHp");
        def.baseQi = ParseInt(obj, "baseQi");
        def.baseSpeed = ParseInt(obj, "baseSpeed");
        def.baseAttack = ParseInt(obj, "baseAttack");
        def.baseDefense = ParseInt(obj, "baseDefense");
        def.initialSkills = ParseStringArray(obj, "initialSkills");
        def.growthCurveId = ParseString(obj, "growthCurveId");
        CharacterDataTables::Instance().RegisterCharacter(def);
    }
    return true;
}

bool CharacterConfigLoader::LoadEquipmentsFromFile(const std::string& filepath) {
    std::string json = ReadFile(filepath);
    if (json.empty()) return false;

    std::string arrayContent = ExtractArray(json, "equipments");
    auto objects = ExtractObjects(arrayContent);

    CharacterDataTables::Instance().ClearEquipments();

    for (const auto& obj : objects) {
        EquipmentDef def;
        def.id = ParseString(obj, "id");
        def.name = ParseString(obj, "name");
        std::string slotStr = ParseString(obj, "slot");
        if (slotStr == "Weapon") def.slot = EquipmentSlot::Weapon;
        else if (slotStr == "Armor") def.slot = EquipmentSlot::Armor;
        else if (slotStr == "Shoes") def.slot = EquipmentSlot::Shoes;
        else if (slotStr == "Accessory") def.slot = EquipmentSlot::Accessory;
        def.bonusAttack = ParseInt(obj, "bonusAttack");
        def.bonusDefense = ParseInt(obj, "bonusDefense");
        def.bonusHp = ParseInt(obj, "bonusHp");
        def.bonusQi = ParseInt(obj, "bonusQi");
        def.bonusSpeed = ParseInt(obj, "bonusSpeed");
        CharacterDataTables::Instance().RegisterEquipment(def);
    }
    return true;
}

bool CharacterConfigLoader::LoadCurvesFromFile(const std::string& filepath) {
    std::string json = ReadFile(filepath);
    if (json.empty()) return false;

    std::string arrayContent = ExtractArray(json, "curves");
    auto objects = ExtractObjects(arrayContent);

    GrowthCurve::ClearCurves();

    for (const auto& obj : objects) {
        CurveDef def;
        def.hpFactor = ParseFloat(obj, "hpFactor", 1.0f);
        def.qiFactor = ParseFloat(obj, "qiFactor", 1.0f);
        def.atkFactor = ParseFloat(obj, "atkFactor", 1.0f);
        def.defFactor = ParseFloat(obj, "defFactor", 1.0f);
        def.spdFactor = ParseFloat(obj, "spdFactor", 1.0f);
        def.baseExp = ParseInt(obj, "baseExp", 100);
        def.expFactor = ParseFloat(obj, "expFactor", 1.5f);
        GrowthCurve::RegisterCurve(ParseString(obj, "id"), def);
    }
    return true;
}

bool CharacterConfigLoader::LoadAllFromDirectory(const std::string& dir) {
    std::string prefix = dir;
    if (!prefix.empty() && prefix.back() != '/' && prefix.back() != '\\') {
        prefix += '/';
    }
    bool ok1 = LoadCharactersFromFile(prefix + "characters.json");
    bool ok2 = LoadEquipmentsFromFile(prefix + "equipments.json");
    bool ok3 = LoadCurvesFromFile(prefix + "curves.json");
    return ok1 && ok2 && ok3;
}
