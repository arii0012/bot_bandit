#pragma once
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "CarDatabase150.hpp"
#include "Procedural3DModeler.hpp"
#include "EngineCore.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

struct AnimInstance {
  ModelAnimation* anims = nullptr;
  int count = 0;
  int current = 0;
  float frame = 0.f;
  float blend = 1.f;
  std::string name;
};

struct LoadedModel {
  Model model{};
  bool valid = false;
  AnimInstance anim{};
  bool fromFile = false;
};

class GlbModelCache {
public:
  LoadedModel& GetOrLoad(const std::string& id, const std::string& path) {
    auto it = cache_.find(id);
    if (it != cache_.end()) return it->second;
    LoadedModel lm;
    if (fs::exists(path)) {
      lm.model = LoadModel(path.c_str());
      if (lm.model.meshCount > 0) {
        lm.valid = true;
        lm.fromFile = true;
        int ac = 0;
        ModelAnimation* a = LoadModelAnimations(path.c_str(), &ac);
        if (a && ac > 0) {
          lm.anim.anims = a;
          lm.anim.count = ac;
          lm.anim.current = 0;
        }
      }
    }
    cache_[id] = lm;
    return cache_[id];
  }

  void SetAnim(LoadedModel& lm, const std::string& name) {
    if (!lm.anim.anims || lm.anim.count <= 0) return;
    for (int i = 0; i < lm.anim.count; ++i) {
      if (lm.anim.anims[i].name && name == lm.anim.anims[i].name) {
        if (lm.anim.current != i) {
          lm.anim.current = i;
          lm.anim.frame = 0;
        }
        lm.anim.name = name;
        return;
      }
    }
  }

  void UpdateAnim(LoadedModel& lm, float dt, float speed = 1.f) {
    if (!lm.valid || !lm.anim.anims || lm.anim.count <= 0) return;
    ModelAnimation& a = lm.anim.anims[lm.anim.current];
    if (a.frameCount <= 0) return;
    lm.anim.frame += dt * 30.f * speed;
    while (lm.anim.frame >= a.frameCount) lm.anim.frame -= a.frameCount;
    UpdateModelAnimation(lm.model, a, (int)lm.anim.frame);
  }

  void Draw(LoadedModel& lm, Vector3 pos, float yawDeg, float scale, Color tint) {
    if (!lm.valid) return;
    DrawModelEx(lm.model, pos, {0, 1, 0}, yawDeg, {scale, scale, scale}, tint);
  }

  void UnloadAll() {
    for (auto& [_, lm] : cache_) {
      if (lm.anim.anims) UnloadModelAnimations(lm.anim.anims, lm.anim.count);
      if (lm.valid && lm.fromFile) UnloadModel(lm.model);
    }
    cache_.clear();
  }

  ~GlbModelCache() { UnloadAll(); }

private:
  std::unordered_map<std::string, LoadedModel> cache_;
};

struct WorldPOI {
  enum Type { Gas, Wash, Impound, Barbershop, Hospital, Club, Bank } type;
  Vector3 pos{};
  std::string label;
};

class ModelsAndWorld3D {
public:
  void Init() {
    // try load character glbs
    const char* chars[] = {
      "male_street","male_cyber","male_suit","male_blond","male_sport",
      "female_model","female_rock","female_blond","female_street","female_vip"
    };
    for (auto* c : chars) {
      std::string path = std::string("assets/models/char_") + c + ".glb";
      cache_.GetOrLoad(c, path);
    }
    const char* cars[] = {"sedan","coupe","suv","hyper","muscle","mercedes","bmw","lambo","lada","tesla"};
    for (auto* c : cars) {
      cache_.GetOrLoad(std::string("car_") + c, std::string("assets/models/car_") + c + ".glb");
    }
    for (int i = 0; i < 20; ++i) {
      char id[32];
      snprintf(id, sizeof(id), "building_%02d", i);
      cache_.GetOrLoad(id, std::string("assets/models/") + id + ".glb");
    }

    // POIs
    pois_ = {
      {WorldPOI::Gas, {25, 0, 10}, "AZS / Gas"},
      {WorldPOI::Gas, {-30, 0, 20}, "Charging"},
      {WorldPOI::Wash, {15, 0, -25}, "Car Wash"},
      {WorldPOI::Impound, {-40, 0, -15}, "Impound"},
      {WorldPOI::Barbershop, {8, 0, 30}, "Barber"},
      {WorldPOI::Hospital, {-12, 0, 35}, "Hospital"},
      {WorldPOI::Club, {35, 0, -5}, "Club"},
      {WorldPOI::Bank, {0, 0, 40}, "Bank"},
    };

    // static buildings placement
    for (int i = 0; i < 20; ++i) {
      float ang = i * 0.31f;
      float r = 18.f + (i % 5) * 6.f;
      buildingPos_.push_back({cosf(ang) * r, 0, sinf(ang) * r});
      buildingSeed_.push_back(i * 17 + 3);
    }
  }

  void SetTimeOfDay(float h) { timeOfDay_ = h; }
  float TimeOfDay() const { return timeOfDay_; }
  bool IsNight() const { return timeOfDay_ < 6.5f || timeOfDay_ > 20.f; }

  void Update(float dt) {
    timeOfDay_ = fmodf(timeOfDay_ + dt * 0.02f, 24.f);

  void DrawCity(const Camera3D& cam, EngineOptimizer& opt) {
    Color ground = IsNight() ? Color{18, 18, 24, 255} : Color{40, 42, 48, 255};
    DrawPlane({0, 0, 0}, {200, 200}, ground);
    DrawGrid(40, 2.f);

    for (int i = 0; i < (int)buildingPos_.size(); ++i) {
      Vector3 p = buildingPos_[i];
      float dist = Vector3Distance(cam.position, p);
      if (!opt.InFrustum(cam, p, 5.f)) { opt.NoteCull(); continue; }
      int lod = opt.LodLevel(dist);
      std::string id = "building_";
      char buf[8];
      snprintf(buf, sizeof(buf), "%02d", i);
      id += buf;
      auto& lm = cache_.GetOrLoad(id, "assets/models/" + id + ".glb");
      if (lm.valid) {
        float sc = lod >= 2 ? 0.9f : 1.f;
        cache_.Draw(lm, p, (float)(i * 20), sc, WHITE);
      } else {
        float h = 4.f + (buildingSeed_[i] % 8);
        if (lod >= 2) h *= 0.7f;
        Procedural3DModeler::DrawBuilding({p.x, h * 0.5f, p.z}, 2.2f, h, 2.f, buildingSeed_[i], IsNight());
      }
      opt.NoteDraw();
    }

    for (auto& poi : pois_) {
      if (!opt.InFrustum(cam, poi.pos, 2.f)) continue;
      Color c = WHITE;
      switch (poi.type) {
        case WorldPOI::Gas: c = {50, 200, 80, 255}; break;
        case WorldPOI::Wash: c = {80, 160, 255, 255}; break;
        case WorldPOI::Impound: c = {200, 80, 60, 255}; break;
        case WorldPOI::Barbershop: c = {220, 180, 60, 255}; break;
        case WorldPOI::Hospital: c = {220, 220, 255, 255}; break;
        case WorldPOI::Club: c = {200, 40, 120, 255}; break;
        case WorldPOI::Bank: c = {244, 196, 48, 255}; break;
      }
      DrawCube(poi.pos, 2.f, 1.5f, 2.f, c);
      DrawCubeWires(poi.pos, 2.f, 1.5f, 2.f, BLACK);
    }
  }

  void DrawCharacter(const std::string& preset, Vector3 pos, float yaw, float animT,
                     CharVisual3D::Anim anim, bool female) {
    auto& lm = cache_.GetOrLoad(preset, "assets/models/char_" + preset + ".glb");
    if (lm.valid) {
      const char* names[] = {"Idle","Walk","Run","Drive","Fight","Shoot","Dance","UsePhone"};
      int idx = 0;
      switch (anim) {
        case CharVisual3D::Anim::Walk: idx = 1; break;
        case CharVisual3D::Anim::Run: idx = 2; break;
        case CharVisual3D::Anim::EnterCar: idx = 3; break;
        case CharVisual3D::Anim::Fight: idx = 4; break;
        case CharVisual3D::Anim::Shoot: idx = 5; break;
        case CharVisual3D::Anim::Dance: idx = 6; break;
        default: idx = 0; break;
      }
      cache_.SetAnim(lm, names[idx]);
      cache_.UpdateAnim(lm, 0.016f);
      cache_.Draw(lm, pos, yaw, 1.f, WHITE);
    } else {
      CharVisual3D v;
      v.female = female;
      v.anim = anim;
      v.animTime = animT;
      Procedural3DModeler::DrawPerson(v, pos, yaw, 1.f);
    }
  }

  void DrawCar(const std::string& bodyId, Color body, Vector3 pos, float yaw, float wheel, bool neon, bool lights) {
    auto& lm = cache_.GetOrLoad("car_" + bodyId, "assets/models/car_" + bodyId + ".glb");
    if (lm.valid) {
      cache_.Draw(lm, pos, yaw, 1.f, body);
    } else {
      Procedural3DModeler::DrawCar3D(body, pos, yaw, wheel, neon, lights, 1.f);
    }
  }

  const std::vector<WorldPOI>& POIs() const { return pois_; }

  WorldPOI* NearestPOI(Vector3 p, float maxDist = 8.f) {
    WorldPOI* best = nullptr;
    float bd = maxDist;
    for (auto& x : pois_) {
      float d = Vector3Distance(p, x.pos);
      if (d < bd) { bd = d; best = &x; }
    }
    return best;
  }

  const std::vector<CarDef>& Cars150() const { return CarDatabase150::All(); }
  const HouseDef* Houses() const { return HouseCatalog(); }

  GlbModelCache& Cache() { return cache_; }

private:
  GlbModelCache cache_;
  std::vector<WorldPOI> pois_;
  std::vector<Vector3> buildingPos_;
  std::vector<int> buildingSeed_;
  float timeOfDay_ = 14.f;
};
