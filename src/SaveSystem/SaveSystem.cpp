#include "SaveSystem/SaveSystem.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "Systems/ReputationScales.hpp"
#include "raylib.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

bool SaveSystem::Exists() const {
  return FileExists(Path().c_str());
}

bool SaveSystem::Save(const Character& p, const World& w) {
  MakeDirectory("data");
  std::string tmp = Path() + ".tmp";
  std::ostringstream o;
  o << "{\n";
  o << "  \"money\": " << p.Money() << ",\n";
  o << "  \"energy\": " << p.Energy() << ",\n";
  o << "  \"level\": " << p.Level() << ",\n";
  o << "  \"wanted\": " << p.Wanted() << ",\n";
  o << "  \"heat\": " << p.Heat() << ",\n";
  o << "  \"city\": \"" << w.CurrentCityId() << "\",\n";
  o << "  \"street\": " << ReputationScales::Get().Street() << ",\n";
  o << "  \"system\": " << ReputationScales::Get().System() << ",\n";
  o << "  \"debt\": " << ReputationScales::Get().Debt() << ",\n";
  o << "  \"cars\": [";
  bool first = true;
  for (auto& c : p.Inv().cars) {
    if (!first) o << ",";
    o << "\"" << c << "\"";
    first = false;
  }
  o << "],\n  \"houses\": [";
  first = true;
  for (auto& h : p.Inv().houses) {
    if (!first) o << ",";
    o << "\"" << h << "\"";
    first = false;
  }
  o << "]\n}\n";

  std::ofstream f(tmp, std::ios::binary);
  if (!f) return false;
  f << o.str();
  f.close();
  // atomic replace
  std::remove(Path().c_str());
  if (std::rename(tmp.c_str(), Path().c_str()) != 0) {
    // fallback copy
    std::ifstream in(tmp, std::ios::binary);
    std::ofstream out(Path(), std::ios::binary);
    out << in.rdbuf();
    std::remove(tmp.c_str());
  }
  return FileExists(Path().c_str());
}

static int ParseIntField(const std::string& s, const char* key, int def) {
  auto pos = s.find(key);
  if (pos == std::string::npos) return def;
  pos = s.find(':', pos);
  if (pos == std::string::npos) return def;
  return atoi(s.c_str() + pos + 1);
}

static long long ParseLLField(const std::string& s, const char* key, long long def) {
  auto pos = s.find(key);
  if (pos == std::string::npos) return def;
  pos = s.find(':', pos);
  if (pos == std::string::npos) return def;
  return atoll(s.c_str() + pos + 1);
}

bool SaveSystem::Load(Character& p, World& w) {
  if (!Exists()) return false;
  char* txt = LoadFileText(Path().c_str());
  if (!txt) return false;
  std::string s(txt);
  UnloadFileText(txt);

  p.AddMoney(ParseLLField(s, "\"money\"", p.Money()) - p.Money());
  // set energy via public API
  int en = ParseIntField(s, "\"energy\"", p.Energy());
  p.SetEnergy(en);
  int wanted = ParseIntField(s, "\"wanted\"", 0);
  p.SetWanted(wanted);
  // heat - AddHeat relative
  int heat = ParseIntField(s, "\"heat\"", 0);
  if (p.Heat() < heat) p.AddHeat(heat - p.Heat());

  int street = ParseIntField(s, "\"street\"", 0);
  int system = ParseIntField(s, "\"system\"", 0);
  int debt = ParseIntField(s, "\"debt\"", 0);
  ReputationScales::Get().Reset();
  ReputationScales::Get().AddStreet(street);
  ReputationScales::Get().AddSystem(system);
  ReputationScales::Get().AddDebt(debt);

  // city
  auto cpos = s.find("\"city\"");
  if (cpos != std::string::npos) {
    auto q1 = s.find('"', cpos + 6);
    auto q2 = s.find('"', q1 + 1);
    if (q1 != std::string::npos && q2 != std::string::npos) {
      std::string city = s.substr(q1 + 1, q2 - q1 - 1);
      w.SetCity(city);
      p.SetCity(city);
    }
  }
  return true;
}
