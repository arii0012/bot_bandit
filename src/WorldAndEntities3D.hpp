#pragma once
#include "ModelsAndWorld3D.hpp"
#include "CoreEngine.hpp"
#include "Procedural3DModeler.hpp"
#include <string>
#include <vector>
#include <cmath>

struct JailCell {
  Vector3 pos{-50.f, 0.f, -40.f};
  bool occupied = false;
  float timer = 0.f;
};

class WorldAndEntities3D {
public:
  void Init(CoreEngine& core) {
    world_.Init();
    int worldId = core.scene.Add(SceneNodeType::World, "root", {0, 0, 0}, {200, 1, 200});
    int dist = core.scene.Add(SceneNodeType::District, "downtown", {0, 0, 0}, {80, 1, 80}, worldId);
    for (int i = 0; i < 20; ++i) {
      float a = i * 0.31f;
      float r = 18.f + (i % 5) * 6.f;
      Vector3 p = {cosf(a) * r, 0, sinf(a) * r};
      core.scene.Add(SceneNodeType::Building, "b_" + std::to_string(i), p, {2.5f, 6.f, 2.5f}, dist);
    }
    core.scene.Add(SceneNodeType::Garage, "hall_0", {40, 0, 0}, {20, 5, 20}, dist);
    core.scene.Add(SceneNodeType::Casino, "casino", {-35, 0, 15}, {15, 6, 15}, dist);
    core.scene.Add(SceneNodeType::Building, "kpz", jail_.pos, {8, 4, 8}, dist);
    core.scene.Add(SceneNodeType::Building, "azs_0", {25, 0, 10}, {4, 3, 4}, dist);
    core.scene.Add(SceneNodeType::Building, "wash_0", {15, 0, -25}, {4, 3, 4}, dist);
    core.scene.Add(SceneNodeType::Building, "impound", {-40, 0, -15}, {6, 3, 6}, dist);
  }

  ModelsAndWorld3D& World() { return world_; }
  JailCell& Jail() { return jail_; }

  void Draw(const Camera3D& cam, CoreEngine& core) {
    if (!std::isfinite(cam.position.x)) return;
    core.optimizer.BeginFrame();
    world_.DrawCity(cam, core.optimizer);
    Procedural3DModeler::DrawBuilding(
        {jail_.pos.x, 2.f, jail_.pos.z}, 6.f, 4.f, 6.f, 99, world_.IsNight());
    if (jail_.occupied) {
      DrawCube({jail_.pos.x, 1.2f, jail_.pos.z}, 1.5f, 2.2f, 1.5f, {60, 60, 70, 255});
    }
  }

  void ArrestPlayer(float minutes) {
    if (minutes < 0.5f) minutes = 0.5f;
    if (minutes > 30.f) minutes = 30.f;
    jail_.occupied = true;
    jail_.timer = minutes * 60.f;
  }

  bool UpdateJail(float dt) {
    if (!jail_.occupied) return false;
    if (dt < 0.f || dt > 0.25f) dt = 0.016f;
    jail_.timer -= dt;
    if (jail_.timer <= 0.f) {
      jail_.occupied = false;
      jail_.timer = 0.f;
      return true;
    }
    return false;
  }

  bool TryEscape() {
    if (!jail_.occupied) return false;
    if (GetRandomValue(0, 100) < 35) {
      jail_.occupied = false;
      jail_.timer = 0.f;
      return true;
    }
    jail_.timer += 30.f;
    return false;
  }

  const std::vector<CarDef>& Cars() const { return world_.Cars150(); }
  const HouseDef* Houses() const { return world_.Houses(); }

private:
  ModelsAndWorld3D world_;
  JailCell jail_;
};
