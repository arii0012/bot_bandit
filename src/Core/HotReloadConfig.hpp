#pragma once
#include "raylib.h"
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

class HotReloadConfig {
public:
  static HotReloadConfig& Get() {
    static HotReloadConfig inst;
    return inst;
  }

  void EnsureDefault(const std::string& path = "data/balance.json") {
    path_ = path;
    if (!FileExists(path.c_str())) {
      const char* def =
          "{\n"
          "  \"work_pay_mult\": 1.0,\n"
          "  \"rob_risk\": 0.35,\n"
          "  \"heist_risk\": 0.55,\n"
          "  \"bribe_base\": 15000,\n"
          "  \"club_entry\": 5000,\n"
          "  \"vip_entry\": 25000,\n"
          "  \"stim_cost\": 15000,\n"
          "  \"jail_seconds_per_star\": 30\n"
          "}\n";
      MakeDirectory("data");
      {
        std::ofstream out(path, std::ios::binary);
        if (out) out << def;
      }
    }
    Reload();
  }

  void Reload() {
    if (!FileExists(path_.c_str())) return;
    char* txt = LoadFileText(path_.c_str());
    if (!txt) return;
    std::string s(txt);
    UnloadFileText(txt);
    workPayMult_ = ParseF(s, "work_pay_mult", 1.f);
    robRisk_ = ParseF(s, "rob_risk", 0.35f);
    heistRisk_ = ParseF(s, "heist_risk", 0.55f);
    bribeBase_ = ParseI(s, "bribe_base", 15000);
    clubEntry_ = ParseI(s, "club_entry", 5000);
    vipEntry_ = ParseI(s, "vip_entry", 25000);
    stimCost_ = ParseI(s, "stim_cost", 15000);
    jailPerStar_ = ParseI(s, "jail_seconds_per_star", 30);
  }

  float WorkPayMult() const { return workPayMult_; }
  float RobRisk() const { return robRisk_; }
  float HeistRisk() const { return heistRisk_; }
  int BribeBase() const { return bribeBase_; }
  int ClubEntry() const { return clubEntry_; }
  int VipEntry() const { return vipEntry_; }
  int StimCost() const { return stimCost_; }
  int JailPerStar() const { return jailPerStar_; }

private:
  static float ParseF(const std::string& s, const char* key, float def) {
    auto pos = s.find(key);
    if (pos == std::string::npos) return def;
    pos = s.find(':', pos);
    if (pos == std::string::npos) return def;
    return static_cast<float>(std::atof(s.c_str() + pos + 1));
  }
  static int ParseI(const std::string& s, const char* key, int def) {
    return static_cast<int>(ParseF(s, key, static_cast<float>(def)));
  }

  std::string path_ = "data/balance.json";
  float workPayMult_ = 1.f, robRisk_ = 0.35f, heistRisk_ = 0.55f;
  int bribeBase_ = 15000, clubEntry_ = 5000, vipEntry_ = 25000, stimCost_ = 15000, jailPerStar_ = 30;
};
