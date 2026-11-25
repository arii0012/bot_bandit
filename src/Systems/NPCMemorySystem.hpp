#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

class NPCMemorySystem {
public:
  static NPCMemorySystem& Get() {
    static NPCMemorySystem inst;
    return inst;
  }

  void Remember(const std::string& npc, const std::string& fact, int delta = 1) {
    mem_[npc + "::" + fact] += delta;
  }

  int Recall(const std::string& npc, const std::string& fact) const {
    auto it = mem_.find(npc + "::" + fact);
    return it == mem_.end() ? 0 : it->second;
  }

  void NoteBetrayal(const std::string& npc) { Remember(npc, "betrayed", 1); }
  void NoteSaved(const std::string& npc) { Remember(npc, "saved", 1); }
  void NoteLied(const std::string& npc) { Remember(npc, "lied", 1); }

  bool WasBetrayed(const std::string& npc) const { return Recall(npc, "betrayed") > 0; }
  bool WasSaved(const std::string& npc) const { return Recall(npc, "saved") > 0; }

  // quote past deed in late acts
  const char* QuoteAboutPlayer(const std::string& npc) const {
    if (WasBetrayed(npc)) return "Ty uzhe sdaval svoikh.";
    if (WasSaved(npc)) return "Ya pomnyu, ty vyta shchil menya.";
    if (Recall(npc, "lied") > 0) return "Ty vral. Ya ne zabyl.";
    return "Poka ty dlya menya - Nol'.";
  }

  void Reset() { mem_.clear(); }

private:
  std::unordered_map<std::string, int> mem_;
};
