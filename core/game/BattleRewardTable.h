#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct DropEntry {
    std::string itemId;
    int count = 1;
    float probability = 1.0f;  // 0.0 ~ 1.0
};

struct BattleReward {
    std::string enemyGroupId;
    int exp = 0;
    int gold = 0;
    std::vector<DropEntry> drops;
};

class BattleRewardTable {
public:
    static BattleRewardTable& Instance();

    bool LoadFromFile(const std::string& filepath);
    bool LoadFromString(const std::string& content);

    const BattleReward* GetReward(const std::string& enemyGroupId) const;
    void Clear();

private:
    BattleRewardTable() = default;
    std::unordered_map<std::string, BattleReward> rewards_;
};
