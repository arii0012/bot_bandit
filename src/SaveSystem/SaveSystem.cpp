#include "SaveSystem/SaveSystem.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "Systems/ReputationScales.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
using json = nlohmann::json;

json SaveColor(const Color& c) {
  return json::array({c.r, c.g, c.b, c.a});
}

Color ReadColor(const json& value, const Color& fallback) {
  if (!value.is_array() || value.size() != 4) return fallback;
  for (const auto& channel : value) {
    if (!channel.is_number_integer()) return fallback;
    const int n = channel.get<int>();
    if (n < 0 || n > 255) return fallback;
  }
  return Color{
    static_cast<unsigned char>(value[0].get<int>()),
    static_cast<unsigned char>(value[1].get<int>()),
    static_cast<unsigned char>(value[2].get<int>()),
    static_cast<unsigned char>(value[3].get<int>())
  };
}

template <typename T>
void ReadIf(const json& root, const char* key, T& target) {
  const auto it = root.find(key);
  if (it != root.end() && !it->is_null()) {
    try { target = it->get<T>(); } catch (...) {}
  }
}

void ReadStringVector(const json& root, const char* key, std::vector<std::string>& target) {
  const auto it = root.find(key);
  if (it == root.end() || !it->is_array()) return;
  std::vector<std::string> values;
  for (const auto& item : *it) {
    if (item.is_string()) values.push_back(item.get<std::string>());
  }
  target = std::move(values);
}

json BuildSave(const Character& p, const World& w) {
  const Inventory& inv = p.Inv();
  json materials = json::object();
  for (const auto& [name, amount] : inv.materials) materials[name] = amount;

  json skills = json::object();
  for (const auto& [name, value] : p.skills_) skills[name] = value;

  return json{
    {"schema_version", 2},
    {"character", {
      {"name", p.name_},
      {"money", std::max<int64_t>(0, p.money_)},
      {"level", p.level_},
      {"exp", p.exp_},
      {"energy", p.energy_},
      {"heat", p.heat_},
      {"wanted", p.wanted_},
      {"strength", p.strength_},
      {"agility", p.agility_},
      {"intelligence", p.intelligence_},
      {"skill_points", p.skillPoints_},
      {"skills", skills},
      {"city", p.cityId_},
      {"active_car", p.activeCar_},
      {"female", p.female_},
      {"hair_style", p.hairStyle_},
      {"body_type", p.bodyType_},
      {"top_style", p.topStyle_},
      {"pants_style", p.pantsStyle_},
      {"hat_style", p.hatStyle_},
      {"shoe_style", p.shoeStyle_},
      {"accessory_style", p.accStyle_},
      {"hair_color", SaveColor(p.hairColor_)},
      {"eyes_color", SaveColor(p.colorEyes_)},
      {"top_color", SaveColor(p.colorTop_)},
      {"pants_color", SaveColor(p.colorPants_)},
      {"hat_color", SaveColor(p.colorHat_)},
      {"shoes_color", SaveColor(p.colorShoes_)},
      {"accessory_color", SaveColor(p.colorAcc_)},
      {"engine_level", p.engineLvl_},
      {"turbo_level", p.turboLvl_},
      {"neon_car", p.neonCar_},
      {"car_color", json::array({p.carR_, p.carG_, p.carB_})},
      {"dirt", p.dirt_},
      {"wet", p.wet_}
    }},
    {"inventory", {
      {"clothes", inv.clothes},
      {"cars", inv.cars},
      {"houses", inv.houses},
      {"gadgets", inv.gadgets},
      {"items", inv.items},
      {"materials", materials}
    }},
    {"world", {
      {"city", w.CurrentCityId()},
      {"street_reputation", ReputationScales::Get().Street()},
      {"system_reputation", ReputationScales::Get().System()},
      {"debt", ReputationScales::Get().Debt()}
    }}
  };
}
} // namespace

bool SaveSystem::Exists() const {
  std::error_code ec;
  return std::filesystem::is_regular_file(Path(), ec) && !ec;
}

bool SaveSystem::Save(const Character& p, const World& w) {
  namespace fs = std::filesystem;
  std::error_code ec;
  fs::create_directories(fs::path(Path()).parent_path(), ec);
  if (ec) return false;

  const std::string tempPath = Path() + ".tmp";
  const std::string backupPath = Path() + ".bak";
  {
    std::ofstream file(tempPath, std::ios::binary | std::ios::trunc);
    if (!file) return false;
    try {
      file << BuildSave(p, w).dump(2) << '\n';
      file.flush();
      if (!file.good()) {
        file.close();
        std::remove(tempPath.c_str());
        return false;
      }
    } catch (...) {
      file.close();
      std::remove(tempPath.c_str());
      return false;
    }
  }

  std::remove(backupPath.c_str());
  const bool hadPrevious = Exists();
  if (hadPrevious && std::rename(Path().c_str(), backupPath.c_str()) != 0) {
    std::remove(tempPath.c_str());
    return false;
  }

  if (std::rename(tempPath.c_str(), Path().c_str()) != 0) {
    if (hadPrevious) std::rename(backupPath.c_str(), Path().c_str());
    std::remove(tempPath.c_str());
    return false;
  }

  std::remove(backupPath.c_str());
  return Exists();
}

bool SaveSystem::Load(Character& p, World& w) {
  json root;
  auto readSave = [](const std::string& path, json& destination) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    try {
      file >> destination;
      return destination.is_object();
    } catch (...) {
      return false;
    }
  };

  // Recover the last known-good snapshot if the primary file was interrupted
  // or damaged during a previous write.
  if (!readSave(Path(), root) && !readSave(Path() + ".bak", root))
    return false;

  // Migrate the original flat save format (schema v1) in memory.
  if (!root.contains("character")) {
    json legacyCharacter = json::object();
    for (const char* key : {"name", "money", "level", "exp", "energy", "heat", "wanted",
                            "strength", "agility", "intelligence", "skill_points", "city",
                            "active_car", "female", "hair_style", "body_type", "top_style",
                            "pants_style", "hat_style", "shoe_style", "accessory_style",
                            "hair_color", "eyes_color", "top_color", "pants_color",
                            "hat_color", "shoes_color", "accessory_color", "engine_level",
                            "turbo_level", "neon_car", "car_color", "dirt", "wet", "skills"}) {
      if (root.contains(key)) legacyCharacter[key] = root[key];
    }
    root["character"] = std::move(legacyCharacter);

    json legacyInventory = json::object();
    for (const char* key : {"cars", "houses"}) {
      if (root.contains(key)) legacyInventory[key] = root[key];
    }
    if (!legacyInventory.empty()) root["inventory"] = std::move(legacyInventory);

    json legacyWorld = json::object();
    for (const char* key : {"city", "street", "system", "debt"}) {
      if (root.contains(key)) {
        const char* destination = key;
        if (std::string(key) == "street") destination = "street_reputation";
        else if (std::string(key) == "system") destination = "system_reputation";
        legacyWorld[destination] = root[key];
      }
    }
    if (!legacyWorld.empty()) root["world"] = std::move(legacyWorld);
  }
  if (!root["character"].is_object()) return false;
  const json& character = root["character"];

  ReadIf(character, "name", p.name_);
  ReadIf(character, "money", p.money_);
  ReadIf(character, "level", p.level_);
  ReadIf(character, "exp", p.exp_);
  ReadIf(character, "energy", p.energy_);
  ReadIf(character, "heat", p.heat_);
  ReadIf(character, "wanted", p.wanted_);
  ReadIf(character, "strength", p.strength_);
  ReadIf(character, "agility", p.agility_);
  ReadIf(character, "intelligence", p.intelligence_);
  ReadIf(character, "skill_points", p.skillPoints_);
  ReadIf(character, "city", p.cityId_);
  ReadIf(character, "active_car", p.activeCar_);
  ReadIf(character, "female", p.female_);
  ReadIf(character, "hair_style", p.hairStyle_);
  ReadIf(character, "body_type", p.bodyType_);
  ReadIf(character, "top_style", p.topStyle_);
  ReadIf(character, "pants_style", p.pantsStyle_);
  ReadIf(character, "hat_style", p.hatStyle_);
  ReadIf(character, "shoe_style", p.shoeStyle_);
  ReadIf(character, "accessory_style", p.accStyle_);
  ReadIf(character, "engine_level", p.engineLvl_);
  ReadIf(character, "turbo_level", p.turboLvl_);
  ReadIf(character, "neon_car", p.neonCar_);
  ReadIf(character, "dirt", p.dirt_);
  ReadIf(character, "wet", p.wet_);

  p.money_ = std::max<int64_t>(0, p.money_);
  p.level_ = std::clamp(p.level_, 1, 9999);
  p.exp_ = std::max(0, p.exp_);
  p.SetEnergy(p.energy_);
  p.heat_ = std::clamp(p.heat_, 0, 100);
  p.SetWanted(p.wanted_);
  p.strength_ = std::clamp(p.strength_, 1, 999);
  p.agility_ = std::clamp(p.agility_, 1, 999);
  p.intelligence_ = std::clamp(p.intelligence_, 1, 999);
  p.skillPoints_ = std::max(0, p.skillPoints_);
  p.engineLvl_ = std::clamp(p.engineLvl_, 0, 10);
  p.turboLvl_ = std::clamp(p.turboLvl_, 0, 10);
  p.SetDirt(p.dirt_);
  p.SetWet(p.wet_);

  p.hairColor_ = ReadColor(character.value("hair_color", json{}), p.hairColor_);
  p.colorEyes_ = ReadColor(character.value("eyes_color", json{}), p.colorEyes_);
  p.colorTop_ = ReadColor(character.value("top_color", json{}), p.colorTop_);
  p.colorPants_ = ReadColor(character.value("pants_color", json{}), p.colorPants_);
  p.colorHat_ = ReadColor(character.value("hat_color", json{}), p.colorHat_);
  p.colorShoes_ = ReadColor(character.value("shoes_color", json{}), p.colorShoes_);
  p.colorAcc_ = ReadColor(character.value("accessory_color", json{}), p.colorAcc_);

  const json carColor = character.value("car_color", json{});
  if (carColor.is_array() && carColor.size() == 3) {
    bool valid = true;
    for (const auto& c : carColor)
      if (!c.is_number_integer() || c.get<int>() < 0 || c.get<int>() > 255) valid = false;
    if (valid) {
      p.carR_ = static_cast<unsigned char>(carColor[0].get<int>());
      p.carG_ = static_cast<unsigned char>(carColor[1].get<int>());
      p.carB_ = static_cast<unsigned char>(carColor[2].get<int>());
    }
  }

  if (character.contains("skills") && character["skills"].is_object()) {
    p.skills_.clear();
    for (auto it = character["skills"].begin(); it != character["skills"].end(); ++it) {
      if (it.value().is_number_integer())
        p.skills_[it.key()] = std::clamp(it.value().get<int>(), 0, 100);
    }
  }

  if (root.contains("inventory") && root["inventory"].is_object()) {
    const json& inventory = root["inventory"];
    ReadStringVector(inventory, "clothes", p.inv_.clothes);
    ReadStringVector(inventory, "cars", p.inv_.cars);
    ReadStringVector(inventory, "houses", p.inv_.houses);
    ReadStringVector(inventory, "gadgets", p.inv_.gadgets);
    ReadStringVector(inventory, "items", p.inv_.items);
    if (inventory.contains("materials") && inventory["materials"].is_object()) {
      p.inv_.materials.clear();
      for (auto it = inventory["materials"].begin(); it != inventory["materials"].end(); ++it) {
        if (it.value().is_number_integer())
          p.inv_.materials[it.key()] = std::max(0, it.value().get<int>());
      }
    }
  }

  const json* worldData = nullptr;
  if (root.contains("world") && root["world"].is_object()) worldData = &root["world"];
  if (worldData) {
    int street = ReputationScales::Get().Street();
    int system = ReputationScales::Get().System();
    int debt = ReputationScales::Get().Debt();
    ReadIf(*worldData, "street_reputation", street);
    ReadIf(*worldData, "system_reputation", system);
    ReadIf(*worldData, "debt", debt);
    ReputationScales::Get().Reset();
    ReputationScales::Get().AddStreet(std::clamp(street, -100, 100));
    ReputationScales::Get().AddSystem(std::clamp(system, -100, 100));
    ReputationScales::Get().AddDebt(std::clamp(debt, -100, 100));
    std::string city = w.CurrentCityId();
    ReadIf(*worldData, "city", city);
    if (!city.empty()) {
      w.SetCity(city);
      p.SetCity(city);
      w.SyncOwnedHouses(p);
    }
  } else if (!p.cityId_.empty()) {
    w.SetCity(p.cityId_);
    w.SyncOwnedHouses(p);
  }

  return true;
}
