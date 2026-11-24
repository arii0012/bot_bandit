#pragma once
#include <string>
#include <unordered_map>
#include <algorithm>

class SmartNPCMemory {
public:
  void Remember(const std::string& npcId, const std::string& fact, int delta = 1) {
    mem_[npcId + ":" + fact] += delta;
  }
  int Recall(const std::string& npcId, const std::string& fact) const {
    auto it = mem_.find(npcId + ":" + fact);
    return it == mem_.end() ? 0 : it->second;
  }
  bool ClubBanned() const { return Recall("club", "debauchery") >= 2; }
  void NoteHelpDistrict() { Remember("district", "help", 1); }
  bool DistrictFriendly() const { return Recall("district", "help") >= 1; }

private:
  std::unordered_map<std::string, int> mem_;
};
