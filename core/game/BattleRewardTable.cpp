#include "BattleRewardTable.h"
#include "../../foundation/JsonValue.h"
#include "../../foundation/JsonUtils.h"
#include <fstream>
#include <sstream>

BattleRewardTable& BattleRewardTable::Instance() {
    static BattleRewardTable inst;
    return inst;
}

bool BattleRewardTable::LoadFromFile(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) return false;
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return LoadFromString(buffer.str());
}

bool BattleRewardTable::LoadFromString(const std::string& content) {
    rewards_.clear();
    JsonValue root;
    if (!JsonUtils::Parse(content, root)) return false;
    if (!root.HasField("rewards")) return false;

    const JsonValue& rewardsObj = root["rewards"];
    auto keys = rewardsObj.GetObjectKeys();

    for (const auto& key : keys) {
        const JsonValue& rv = rewardsObj[key];
        BattleReward br;
        br.enemyGroupId = key;
        if (rv.HasField("exp")) br.exp = rv["exp"].AsInt();
        if (rv.HasField("gold")) br.gold = rv["gold"].AsInt();
        if (rv.HasField("drops") && rv["drops"].GetType() == JsonType::Array) {
            size_t dcount = rv["drops"].ArraySize();
            for (size_t i = 0; i < dcount; ++i) {
                const JsonValue& dv = rv["drops"][i];
                DropEntry de;
                de.itemId = dv["itemId"].AsString();
                de.count = dv.HasField("count") ? dv["count"].AsInt() : 1;
                de.probability = dv.HasField("probability") ? static_cast<float>(dv["probability"].AsDouble()) : 1.0f;
                br.drops.push_back(de);
            }
        }
        rewards_[key] = std::move(br);
    }
    return true;
}

const BattleReward* BattleRewardTable::GetReward(const std::string& enemyGroupId) const {
    auto it = rewards_.find(enemyGroupId);
    if (it != rewards_.end()) return &it->second;
    return nullptr;
}

void BattleRewardTable::Clear() {
    rewards_.clear();
}
