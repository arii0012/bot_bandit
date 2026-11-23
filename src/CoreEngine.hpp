#pragma once
#include "EngineCore.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <cstdint>

using json = nlohmann::json;

enum class SceneNodeType {
  World, District, Building, Garage, Casino, Dynamic, Light, Camera
};

struct SceneNode {
  int id = 0;
  SceneNodeType type = SceneNodeType::Dynamic;
  std::string name;
  Vector3 pos{};
  Vector3 size{1, 1, 1};
  int parent = -1;
  std::vector<int> children;
  bool visible = true;
  int lodBias = 0;
};

class SceneGraph3D {
public:
  int Add(SceneNodeType type, const std::string& name, Vector3 pos, Vector3 size, int parent = -1) {
    SceneNode n;
    n.id = next_++;
    n.type = type;
    n.name = name;
    n.pos = pos;
    n.size = size;
    n.parent = parent;
    nodes_[n.id] = n;
    if (parent >= 0 && nodes_.count(parent)) nodes_[parent].children.push_back(n.id);
    return n.id;
  }

  SceneNode* Get(int id) {
    auto it = nodes_.find(id);
    return it == nodes_.end() ? nullptr : &it->second;
  }

  void CollectVisible(const Camera3D& cam, EngineOptimizer& opt, std::vector<int>& out) {
    out.clear();
    for (auto& [id, n] : nodes_) {
      if (!n.visible) continue;
      if (n.type == SceneNodeType::World || n.type == SceneNodeType::Camera) continue;
      if (!opt.InFrustum(cam, n.pos, Vector3Length(n.size) * 0.5f)) {
        opt.NoteCull();
        continue;
      }
      out.push_back(id);
      opt.NoteDraw();
    }
  }

  void SetHallStreaming(int hallId, const std::vector<int>& hallNodeIds) {
    for (auto& [id, n] : nodes_) {
      if (n.type == SceneNodeType::Garage || n.name.find("hall_") == 0)
        n.visible = false;
    }
    for (int id : hallNodeIds) {
      if (nodes_.count(id)) nodes_[id].visible = true;
    }
    (void)hallId;
  }

  const std::unordered_map<int, SceneNode>& Nodes() const { return nodes_; }

private:
  std::unordered_map<int, SceneNode> nodes_;
  int next_ = 1;
};

struct DBCar {
  std::string id;
  std::string brand;
  std::string model;
  int price = 0;
  int owner = -1;
  unsigned char r = 200, g = 40, b = 40;
  int engine = 0, turbo = 0;
  bool neon = false;
};

struct DBProperty {
  std::string id;
  std::string name;
  int price = 0;
  int owner = -1;
  int type = 0;
};

struct DBBusiness {
  std::string id;
  std::string name;
  int price = 0;
  int owner = -1;
  int income = 0;
  int staff = 0;
  int risk = 0;
};

struct DBItem {
  std::string id;
  std::string name;
  int qty = 0;
  bool illegal = false;
};

struct DBNPC {
  std::string id;
  std::string name;
  Vector3 pos{};
  int relation = 0;
};

struct DBCrime {
  std::string id;
  std::string desc;
  int stars = 0;
  double time = 0;
};

struct DBMarket {
  std::string id;
  float price = 0;
  float volatility = 0.1f;
};

class GameDatabase {
public:
  std::unordered_map<std::string, DBCar> cars;
  std::unordered_map<std::string, DBProperty> properties;
  std::unordered_map<std::string, DBBusiness> businesses;
  std::unordered_map<std::string, DBItem> items;
  std::unordered_map<std::string, DBNPC> npcs;
  std::vector<DBCrime> crimes;
  std::unordered_map<std::string, DBMarket> market;

  void SeedDefaults() {
    if (market.empty()) {
      market["crypto"] = {"crypto", 100.f, 0.2f};
      market["fuel92"] = {"fuel92", 50.f, 0.05f};
      market["fuel95"] = {"fuel95", 60.f, 0.05f};
      market["fuel100"] = {"fuel100", 75.f, 0.05f};
    }
  }

  json ToJson() const {
    json j;
    j["cars"] = json::object();
    for (auto& [k, c] : cars) {
      j["cars"][k] = {{"brand", c.brand}, {"model", c.model}, {"price", c.price}, {"owner", c.owner},
                      {"color", {c.r, c.g, c.b}}, {"engine", c.engine}, {"turbo", c.turbo}, {"neon", c.neon}};
    }
    j["properties"] = json::object();
    for (auto& [k, p] : properties)
      j["properties"][k] = {{"name", p.name}, {"price", p.price}, {"owner", p.owner}, {"type", p.type}};
    j["businesses"] = json::object();
    for (auto& [k, b] : businesses)
      j["businesses"][k] = {{"name", b.name}, {"price", b.price}, {"owner", b.owner},
                            {"income", b.income}, {"staff", b.staff}, {"risk", b.risk}};
    j["items"] = json::object();
    for (auto& [k, it] : items)
      j["items"][k] = {{"name", it.name}, {"qty", it.qty}, {"illegal", it.illegal}};
    j["crimes"] = json::array();
    for (auto& c : crimes)
      j["crimes"].push_back({{"id", c.id}, {"desc", c.desc}, {"stars", c.stars}, {"time", c.time}});
    j["market"] = json::object();
    for (auto& [k, m] : market)
      j["market"][k] = {{"price", m.price}, {"volatility", m.volatility}};
    return j;
  }

  void FromJson(const json& j) {
    cars.clear(); properties.clear(); businesses.clear(); items.clear(); crimes.clear(); market.clear();
    if (j.contains("cars"))
      for (auto it = j["cars"].begin(); it != j["cars"].end(); ++it) {
        DBCar c;
        c.id = it.key();
        c.brand = it.value().value("brand", "");
        c.model = it.value().value("model", "");
        c.price = it.value().value("price", 0);
        c.owner = it.value().value("owner", -1);
        if (it.value().contains("color") && it.value()["color"].is_array()) {
          c.r = it.value()["color"][0]; c.g = it.value()["color"][1]; c.b = it.value()["color"][2];
        }
        c.engine = it.value().value("engine", 0);
        c.turbo = it.value().value("turbo", 0);
        c.neon = it.value().value("neon", false);
        cars[c.id] = c;
      }
    if (j.contains("market"))
      for (auto it = j["market"].begin(); it != j["market"].end(); ++it) {
        DBMarket m;
        m.id = it.key();
        m.price = it.value().value("price", 0.f);
        m.volatility = it.value().value("volatility", 0.1f);
        market[m.id] = m;
      }
  }
};

using RenderOptimizer = EngineOptimizer;

class CoreEngine {
public:
  SceneGraph3D scene;
  GameDatabase db;
  SaveSystemAtomic saves{"data/save.json"};
  PhysicsWorld physics;
  ParticleSystem3D particles;
  RenderOptimizer optimizer;

  void Init() {
    db.SeedDefaults();
    scene.Add(SceneNodeType::World, "world", {0, 0, 0}, {200, 1, 200}, -1);
  }

  bool SaveGame(const json& playerExtra) {
    json root = db.ToJson();
    root["player"] = playerExtra;
    return saves.Save(root);
  }

  bool LoadGame(json& playerExtra) {
    json root;
    if (!saves.Load(root)) return false;
    db.FromJson(root);
    if (root.contains("player")) playerExtra = root["player"];
    return true;
  }
};
