#include "PartyManager.h"
#include <sstream>

// 辅助解析函数（匿名命名空间）
namespace {
    int ParseIntFieldPM(const std::string& json, const std::string& key) {
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

    std::string ParseStringFieldPM(const std::string& json, const std::string& key) {
        size_t pos = json.find(key);
        if (pos == std::string::npos) return "";
        size_t quote = json.find("\"", pos + key.length());
        if (quote == std::string::npos) return "";
        size_t start = quote + 1;
        size_t end = json.find("\"", start);
        if (end == std::string::npos) return "";
        return json.substr(start, end - start);
    }
}

int PartyManager::AddMember(const std::string& characterId, int level) {
    members.push_back(std::make_unique<PartyMember>(characterId, level));
    return static_cast<int>(members.size()) - 1;
}

void PartyManager::RemoveMember(int index) {
    if (index >= 0 && index < static_cast<int>(members.size())) {
        members.erase(members.begin() + index);
    }
}

void PartyManager::SetMemberActive(int index, bool active) {
    auto m = GetMember(index);
    if (m) m->SetActive(active);
}

void PartyManager::SwapMemberOrder(int indexA, int indexB) {
    if (indexA >= 0 && indexA < static_cast<int>(members.size()) &&
        indexB >= 0 && indexB < static_cast<int>(members.size())) {
        std::swap(members[indexA], members[indexB]);
    }
}

std::vector<PartyMember*> PartyManager::GetActiveMembers() const {
    std::vector<PartyMember*> result;
    for (const auto& m : members) {
        if (m->IsActive()) {
            result.push_back(m.get());
        }
    }
    return result;
}

std::vector<CombatantInitData> PartyManager::ExportActiveMembersForBattle() const {
    std::vector<CombatantInitData> result;
    for (const auto& m : members) {
        if (m->IsActive() && !m->IsDead()) {
            result.push_back(m->ExportForBattle());
        }
    }
    return result;
}

void PartyManager::SyncAfterBattle(const std::vector<BattleMemberState>& results) {
    for (const auto& state : results) {
        for (auto& m : members) {
            if (m->GetCharacterId() == state.characterId) {
                m->SyncAfterBattle(state.remainingHp, state.remainingQi, state.isDead);
                m->GainExp(state.expGained);
                break;
            }
        }
    }
}

PartyMember* PartyManager::GetMember(int index) {
    if (index >= 0 && index < static_cast<int>(members.size())) {
        return members[index].get();
    }
    return nullptr;
}

int PartyManager::GetMemberCount() const {
    return static_cast<int>(members.size());
}

void PartyManager::Serialize(std::string& out) const {
    std::stringstream ss;
    ss << "{\"members\":[";
    for (size_t i = 0; i < members.size(); ++i) {
        if (i > 0) ss << ",";
        std::string memberStr;
        members[i]->Serialize(memberStr);
        ss << memberStr;
    }
    ss << "]}";
    out = ss.str();
}

void PartyManager::Deserialize(const std::string& in) {
    members.clear();
    size_t arrPos = in.find("\"members\":[");
    if (arrPos == std::string::npos) return;

    size_t pos = arrPos + 11; // 跳过 `"members":[`
    while (pos < in.size()) {
        // 跳过空白和逗号
        while (pos < in.size() && (in[pos] == ' ' || in[pos] == '\t' || in[pos] == '\n' || in[pos] == '\r' || in[pos] == ',')) ++pos;
        if (pos >= in.size() || in[pos] == ']') break;
        if (in[pos] != '{') { ++pos; continue; }

        size_t objStart = pos;
        // 找匹配的 }
        size_t objEnd = objStart + 1;
        int braceCount = 1;
        while (objEnd < in.size() && braceCount > 0) {
            if (in[objEnd] == '{') ++braceCount;
            else if (in[objEnd] == '}') --braceCount;
            ++objEnd;
        }
        if (braceCount != 0) break;
        --objEnd; // 指向 '}'

        std::string objStr = in.substr(objStart, objEnd - objStart + 1);
        std::string cid = ParseStringFieldPM(objStr, "\"cid\"");
        int lv = ParseIntFieldPM(objStr, "\"lv\"");

        if (!cid.empty()) {
            auto member = std::make_unique<PartyMember>(cid, lv > 0 ? lv : 1);
            member->Deserialize(objStr);
            members.push_back(std::move(member));
        }

        pos = objEnd + 1;
    }
}
