#include "SaveSystem.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
using json = nlohmann::json;

bool SaveSystem::Exists() const {
  return fs::exists(Path());
}

bool SaveSystem::Save(const Character& p, const World& w) {
  fs::create_directories("data");
  json j;
  j["name"] = p.Name();
  j["money"] = p.Money();
  j["level"] = p.Level();
  j["exp"] = p.Exp();
  j["energy"] = p.Energy();
  j["heat"] = p.Heat();
  j["wanted"] = p.Wanted();
  j["strength"] = p.Strength();
  j["agility"] = p.Agility();
  j["intelligence"] = p.Intelligence();
  j["city"] = w.CurrentCityId();
  j["female"] = p.GenderFemale();
  j["hairStyle"] = p.HairStyle();
  j["skills"] = p.skills_;
  j["materials"] = p.Inv().materials;
  j["weapons"] = p.Inv().weapons;
  j["items"] = p.Inv().items;
  j["cars"] = p.Inv().cars;
  j["activeCar"] = p.ActiveCar();

  std::ofstream f(Path());
  if (!f) return false;
  f << j.dump(2);
  return true;
}

bool SaveSystem::Load(Character& p, World& w) {
  std::ifstream f(Path());
  if (!f) return false;
  json j;
  try { f >> j; } catch (...) { return false; }

  if (j.contains("name")) p.SetName(j["name"].get<std::string>());
  if (j.contains("money")) p.money_ = j["money"].get<int64_t>();
  if (j.contains("level")) p.level_ = j["level"].get<int>();
  if (j.contains("exp")) p.exp_ = j["exp"].get<int>();
  if (j.contains("energy")) p.energy_ = j["energy"].get<int>();
  if (j.contains("heat")) p.heat_ = j["heat"].get<int>();
  if (j.contains("wanted")) p.wanted_ = j["wanted"].get<int>();
  if (j.contains("strength")) p.strength_ = j["strength"].get<int>();
  if (j.contains("agility")) p.agility_ = j["agility"].get<int>();
  if (j.contains("intelligence")) p.intelligence_ = j["intelligence"].get<int>();
  if (j.contains("city")) {
    std::string cid = j["city"].get<std::string>();
    w.SetCity(cid);
    p.SetCity(cid);
  }
  if (j.contains("female")) p.SetFemale(j["female"].get<bool>());
  if (j.contains("hairStyle")) p.SetHairStyle(j["hairStyle"].get<int>());
  if (j.contains("skills")) p.skills_ = j["skills"].get<std::map<std::string,int>>();
  if (j.contains("materials")) p.Inv().materials = j["materials"].get<std::map<std::string,int>>();
  if (j.contains("weapons")) p.Inv().weapons = j["weapons"].get<std::vector<std::string>>();
  if (j.contains("items")) p.Inv().items = j["items"].get<std::vector<std::string>>();
  if (j.contains("cars")) p.Inv().cars = j["cars"].get<std::vector<std::string>>();
  if (j.contains("activeCar")) p.SetActiveCar(j["activeCar"].get<std::string>());
  return true;
}
