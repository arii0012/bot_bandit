#include "World.hpp"
#include "Core/Config.hpp"
#include <cmath>
#include <cstdlib>

World::World() {
  buildings_.reserve(16);
}

void World::EnsureBuildings(int sw) {
  if (lastSw_ == sw && !buildings_.empty()) return;
  lastSw_ = sw;
  buildings_.clear();
  // fixed layout — no per-frame allocation, few draw calls
  const int count = 10;
  for (int i = 0; i < count; ++i) {
    Building b;
    b.baseX = (float)i / count;
    b.width = 70 + (i * 13) % 50;
    b.height = 90 + (i * 41) % 140;
    b.seed = i * 17 + 3;
    buildings_.push_back(b);
  }
}

void World::Update(float dt) {
  timeOfDay_ += dt * 0.12f;
  if (timeOfDay_ >= 24.f) timeOfDay_ -= 24.f;
  scroll_ += dt * 8.f; // slow parallax
}

std::string World::CurrentCityName() const {
  auto* c = FindCity(cityId_);
  return c ? c->name : cityId_;
}

const City* World::Current() const { return FindCity(cityId_); }

void World::SetCity(const std::string& id) {
  if (FindCity(id)) cityId_ = id;
}

void World::DrawSky(int sw, int sh) {
  float t = timeOfDay_;
  if (fabsf(t - lastSkyT_) > 0.05f) {
    lastSkyT_ = t;
    if (t < 6.f || t > 21.f) {
      skyTop_ = {10, 10, 25, 255};
      skyBot_ = {20, 15, 35, 255};
    } else if (t < 8.f || t > 19.f) {
      skyTop_ = {80, 40, 60, 255};
      skyBot_ = {120, 60, 40, 255};
    } else {
      skyTop_ = {40, 60, 90, 255};
      skyBot_ = {70, 90, 110, 255};
    }
  }
  int mid = sh / 2;
  DrawRectangleGradientV(0, 0, sw, mid, skyTop_, skyBot_);
  DrawRectangle(0, mid, sw, sh - mid, skyBot_);
}

void World::DrawBuildings(int sw, int sh) {
  EnsureBuildings(sw);
  const int groundY = sh * 2 / 3;
  const float cycle = (float)sw + 120.f;

  for (auto& b : buildings_) {
    float x = fmodf(b.baseX * cycle + scroll_, cycle) - 60.f;
    int bh = b.height;
    int by = groundY - bh;
    int bw = b.width;

    // body — one rect
    DrawRectangle((int)x, by, bw, bh, Color{25, 25, 32, 255});

    // windows: max 3 rows x 3 cols (was many more) — fewer draw calls
    int rows = 2 + (b.seed % 2);
    int cols = 2 + (b.seed % 2);
    int cellW = (bw - 12) / cols;
    int cellH = 10;
    int gapY = (bh - 20) / (rows + 1);
    bool night = timeOfDay_ < 7.f || timeOfDay_ > 20.f;
    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        int wx = (int)x + 6 + c * cellW;
        int wy = by + 12 + r * gapY;
        Color wc = (night && ((b.seed + r * 3 + c) % 4 == 0))
          ? Color{200, 180, 80, 200}
          : Color{40, 45, 55, 255};
        DrawRectangle(wx, wy, cellW - 4, cellH, wc);
      }
    }
  }
}

void World::DrawGround(int sw, int sh) {
  int gy = sh * 2 / 3;
  DrawRectangle(0, gy, sw, sh - gy, Color{30, 30, 35, 255});
  // single road line instead of many details
  DrawRectangle(0, gy + 8, sw, 3, Color{50, 50, 55, 255});
}

void World::Draw(int sw, int sh) {
  DrawSky(sw, sh);
  DrawBuildings(sw, sh);
  DrawGround(sw, sh);

  auto* c = Current();
  if (c) {
    DrawText(c->name.c_str(), 24, sh * 2 / 3 + 20, 28, Config::ColText());
    DrawText(c->desc.c_str(), 24, sh * 2 / 3 + 52, 18, Config::ColMuted());
  }
}
