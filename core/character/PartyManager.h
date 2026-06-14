#pragma once
#include "../../foundation/ISerializable.h"
#include "PartyMember.h"
#include <vector>
#include <memory>

// 战后成员状态（由战斗系统传入）
struct BattleMemberState {
    std::string characterId;
    int remainingHp = 0;
    int remainingQi = 0;
    bool isDead = false;
    int expGained = 0;
};

// 队伍队列管理器
class PartyManager : public ISerializable {
public:
    int AddMember(const std::string& characterId, int level = 1);
    void RemoveMember(int index);
    void SetMemberActive(int index, bool active);
    void SwapMemberOrder(int indexA, int indexB);

    std::vector<PartyMember*> GetActiveMembers() const;
    std::vector<CombatantInitData> ExportActiveMembersForBattle() const;
    void SyncAfterBattle(const std::vector<BattleMemberState>& results);

    PartyMember* GetMember(int index);
    int GetMemberCount() const;

    void Serialize(std::string& out) const override;
    void Deserialize(const std::string& in) override;

private:
    std::vector<std::unique_ptr<PartyMember>> members;
};
