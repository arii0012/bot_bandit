#include "World/World.hpp"
#include "Character/Character.hpp"
#include "Core/FontUI.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "raymath.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <random>
#include <cstdint>

World::World() { EnsureCity(); }

void World::EnsureCity() {
  if (ready_) return;
  buildings3_.clear();
  traffic_.clear();

  // Use a city-specific local generator so the same city keeps the same
  // buildings and property indices after travel or loading a save.
  std::uint32_t seed = 2166136261u;
  for (unsigned char ch : cityId_) {
    seed ^= static_cast<std::uint32_t>(ch);
    seed *= 16777619u;
  }
  std::mt19937 rng(seed);
  auto range = [&rng](int minValue, int maxValue) {
    return std::uniform_int_distribution<int>(minValue, maxValue)(rng);
  };

  int id = 0;
  for (int z = -4; z <= 4; ++z) {
    for (int x = -5; x <= 5; ++x) {
      if ((x == 0 && (z == 0 || z == 1)) || (z == 0 && abs(x) <= 1)) continue;
      Building3D b{};
      b.pos = {static_cast<float>(x) * 8.f + range(-10, 10) / 10.f,
               0.f,
               static_cast<float>(z) * 10.f + range(-10, 10) / 10.f};
      b.w = 3.5f + range(0, 20) / 10.f;
      b.d = 3.5f + range(0, 20) / 10.f;
      b.h = 6.f + range(0, 40) / 5.f;
      b.wall = Color{static_cast<unsigned char>(range(40, 90)),
                     static_cast<unsigned char>(range(40, 90)),
                     static_cast<unsigned char>(range(50, 100)), 255};
      b.price = 250000 + range(0, 40) * 25000;
      b.owned = false;
      buildings3_.push_back(b);
      if (++id >= 36) break;
    }
    if (id >= 36) break;
  }

  for (int i = 0; i < 12; ++i) {
    TrafficCar car{};
    car.pos = {-30.f + i * 6.f, 0.f, (i % 2 == 0) ? 0.f : 3.5f};
    car.speed = 6.f + range(0, 40) / 10.f;
    car.yaw = (i % 2 == 0) ? 0.f : 3.14159f;
    car.body = Color{static_cast<unsigned char>(range(40, 220)),
                     static_cast<unsigned char>(range(40, 220)),
                     static_cast<unsigned char>(range(40, 220)), 255};
    traffic_.push_back(car);
  }
  ready_ = true;
}

void World::Update(float dt) {
  EnsureCity();
  timeOfDay_ = fmodf(timeOfDay_ + dt * 0.02f, 24.f);
  for (auto& c : traffic_) {
    float dir = (c.yaw < 1.f) ? 1.f : -1.f;
    c.pos.x += dir * c.speed * dt;
    c.pos.y = 0.f; // glue to asphalt
    if (c.pos.x > 40.f) c.pos.x = -40.f;
    if (c.pos.x < -40.f) c.pos.x = 40.f;
  }
}

void World::DrawSky3D() {
  // gradient sky via large cube faces not needed in Mode3D — ClearBackground outside
}

void World::DrawGround() {
  // asphalt plane Y=0
  DrawPlane(Vector3{0, 0, 0}, Vector2{120, 120}, Color{32, 32, 38, 255});
  // road strip
  DrawCube(Vector3{0, 0.02f, 0}, 80.f, 0.04f, 6.f, Color{28, 28, 32, 255});
  DrawCube(Vector3{0, 0.02f, 8.f}, 80.f, 0.04f, 6.f, Color{28, 28, 32, 255});
  // lane marks
  for (int i = -20; i <= 20; ++i) {
    DrawCube(Vector3{(float)i * 3.f, 0.03f, 0.f}, 1.2f, 0.02f, 0.15f, Color{200, 200, 80, 200});
    DrawCube(Vector3{(float)i * 3.f, 0.03f, 8.f}, 1.2f, 0.02f, 0.15f, Color{200, 200, 80, 200});
  }
  // sidewalks
  DrawCube(Vector3{0, 0.05f, -4.f}, 80.f, 0.08f, 2.f, Color{55, 55, 60, 255});
  DrawCube(Vector3{0, 0.05f, 12.f}, 80.f, 0.08f, 2.f, Color{55, 55, 60, 255});
}

void World::DrawBuildings3D() {
  for (size_t i = 0; i < buildings3_.size(); ++i) {
    auto& b = buildings3_[i];
    Vector3 center = {b.pos.x, b.h * 0.5f, b.pos.z};
    DrawCube(center, b.w, b.h, b.d, b.wall);
    DrawCubeWires(center, b.w, b.h, b.d, Fade(BLACK, 0.4f));
    // windows
    int floors = (int)(b.h / 2.2f);
    for (int f = 0; f < floors; ++f) {
      for (int w = 0; w < 3; ++w) {
        float wx = b.pos.x - b.w * 0.3f + w * (b.w * 0.3f);
        float wy = 1.2f + f * 2.2f;
        float wz = b.pos.z + b.d * 0.51f;
        const bool night = timeOfDay_ >= 18.f || timeOfDay_ < 6.f;
        const bool lit = ((i * 7 + f * 3 + w * 5) % 4) == 0;
        Color win = (night && lit) ? Color{255, 220, 120, 220}
                                   : Color{30, 40, 60, 200};
        DrawCube(Vector3{wx, wy, wz}, 0.5f, 0.7f, 0.05f, win);
      }
    }
    // door marker
    DrawCube(Vector3{b.pos.x, 0.9f, b.pos.z + b.d * 0.52f}, 0.8f, 1.8f, 0.1f, Color{40, 25, 20, 255});
    if (!b.owned)
      DrawSphere(Vector3{b.pos.x, 2.4f, b.pos.z + b.d * 0.6f}, 0.25f, Color{0, 255, 120, 180});
    else
      DrawSphere(Vector3{b.pos.x, 2.4f, b.pos.z + b.d * 0.6f}, 0.25f, Color{0, 180, 255, 180});
  }
}

void World::DrawTraffic3D() {
  for (auto& c : traffic_) {
    c.pos.y = 0.f;
    // body on road
    DrawCube(Vector3{c.pos.x, 0.45f, c.pos.z}, 2.2f, 0.7f, 1.1f, c.body);
    DrawCube(Vector3{c.pos.x, 0.85f, c.pos.z}, 1.2f, 0.45f, 1.0f, Fade(c.body, 0.85f));
    // wheels on Y~0.2
    DrawCylinder(Vector3{c.pos.x - 0.7f, 0.2f, c.pos.z - 0.55f}, 0.2f, 0.2f, 0.15f, 8, BLACK);
    DrawCylinder(Vector3{c.pos.x + 0.7f, 0.2f, c.pos.z - 0.55f}, 0.2f, 0.2f, 0.15f, 8, BLACK);
    DrawCylinder(Vector3{c.pos.x - 0.7f, 0.2f, c.pos.z + 0.55f}, 0.2f, 0.2f, 0.15f, 8, BLACK);
    DrawCylinder(Vector3{c.pos.x + 0.7f, 0.2f, c.pos.z + 0.55f}, 0.2f, 0.2f, 0.15f, 8, BLACK);
  }
}

void World::DrawProps() {
  // street lamps
  for (int i = -8; i <= 8; i += 2) {
    float x = (float)i * 5.f;
    DrawCylinder(Vector3{x, 2.5f, -3.2f}, 0.08f, 0.08f, 5.f, 6, Color{50, 50, 55, 255});
    DrawSphere(Vector3{x, 5.1f, -3.2f}, 0.25f, Color{255, 230, 150, 200});
  }
  // neon club sign
  DrawCube(Vector3{12.f, 4.f, 15.f}, 6.f, 1.2f, 0.2f, Color{220, 40, 80, 255});
}

void World::Draw3DScene(Camera3D cam) {
  EnsureCity();
  BeginMode3D(cam);
  DrawGround();
  DrawBuildings3D();
  DrawTraffic3D();
  DrawProps();
  EndMode3D();
  const std::string cityLabel = CurrentCityName();
  FontUI::Get().Draw(cityLabel.c_str(), 18.f, 18.f, 20.f, Color{235, 240, 250, 255});
}

void World::Draw(int sw, int sh) {
  // 2D fallback path: dark sky, no blue void
  Color top = timeOfDay_ > 18.f || timeOfDay_ < 6.f ? Color{8, 10, 28, 255} : Color{40, 90, 160, 255};
  DrawRectangle(0, 0, sw, sh, top);
  // simple horizon
  DrawRectangle(0, sh * 2 / 3, sw, sh / 3, Color{30, 30, 36, 255});
}

std::string World::CurrentCityName() const {
  auto* c = FindCity(cityId_);
  return c ? c->name : cityId_;
}

const City* World::Current() const { return FindCity(cityId_); }

void World::SetCity(const std::string& id) {
  if (FindCity(id)) {
    cityId_ = id;
    ready_ = false;
    EnsureCity();
  }
}

int World::NearHouseIndex(Vector3 playerPos, float radius) const {
  for (size_t i = 0; i < buildings3_.size(); ++i) {
    float dx = buildings3_[i].pos.x - playerPos.x;
    float dz = buildings3_[i].pos.z - playerPos.z;
    if (dx * dx + dz * dz < radius * radius) return (int)i;
  }
  return -1;
}

bool World::TryBuyHouse(int idx, Character& p, std::string& msg) {
  if (idx < 0 || idx >= (int)buildings3_.size()) { msg = "Нет дома"; return false; }
  auto& b = buildings3_[idx];
  const std::string propertyId = cityId_ + "_house_" + std::to_string(idx);
  if (b.owned) { msg = "Уже куплен"; return false; }
  if (p.Money() < b.price) {
    msg = std::string("Нужно ") + ArtworkRenderer::FormatRub(b.price);
    return false;
  }
  p.AddMoney(-b.price);
  b.owned = true;
  p.Inv().houses.push_back(propertyId);
  msg = "Дом куплен!";
  return true;
}

bool World::EnterHouse(int idx) {
  if (idx < 0 || idx >= (int)buildings3_.size()) return false;
  if (!buildings3_[idx].owned) return false;
  interiorHouse_ = idx;
  return true;
}

void World::SyncOwnedHouses(const Character& p) {
  const auto& owned = p.Inv().houses;
  for (size_t i = 0; i < buildings3_.size(); ++i) {
    const std::string propertyId = cityId_ + "_house_" + std::to_string(i);
    const std::string legacyId = "house_" + std::to_string(i);
    buildings3_[i].owned =
      std::find(owned.begin(), owned.end(), propertyId) != owned.end() ||
      std::find(owned.begin(), owned.end(), legacyId) != owned.end();
  }
}
