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
  for (int i = 0; i < 12; ++i) {
    Building b;
    b.baseX = (float)i / 12.f;
    b.width = 64 + (i * 17) % 55;
    b.height = 100 + (i * 43) % 160;
    b.seed = i * 19 + 7;
    buildings_.push_back(b);
  }
}

void World::Update(float dt) {
  timeOfDay_ += dt * 0.12f;
  if (timeOfDay_ >= 24.f) timeOfDay_ -= 24.f;
  scroll_ += dt * 10.f;
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
      skyTop_ = {8, 8, 22, 255};
      skyBot_ = {18, 12, 32, 255};
    } else if (t < 8.f || t > 19.f) {
      skyTop_ = {90, 45, 65, 255};
      skyBot_ = {140, 70, 45, 255};
    } else {
      skyTop_ = {55, 95, 140, 255};
      skyBot_ = {100, 140, 170, 255};
    }
  }
  int mid = sh / 2;
  DrawRectangleGradientV(0, 0, sw, mid, skyTop_, skyBot_);
  DrawRectangle(0, mid, sw, sh - mid, skyBot_);
  // sun/moon
  if (t >= 6.f && t <= 20.f) {
    float sunX = (t - 6.f) / 14.f * sw;
    float sunY = mid - sinf((t - 6.f) / 14.f * 3.14159f) * (mid * 0.55f);
    DrawCircle((int)sunX, (int)sunY, 28, Fade(Color{255, 230, 150, 255}, 0.9f));
    DrawCircle((int)sunX, (int)sunY, 40, Fade(Color{255, 200, 100, 255}, 0.15f));
  } else {
    DrawCircle(sw * 3 / 4, sh / 6, 18, Color{220, 220, 240, 255});
  }
}

void World::DrawBuildings(int sw, int sh) {
  EnsureBuildings(sw);
  const int groundY = sh * 2 / 3;
  const float cycle = (float)sw + 140.f;

  for (auto& b : buildings_) {
    float x = fmodf(b.baseX * cycle + scroll_, cycle) - 70.f;
    int bh = b.height;
    int by = groundY - bh;
    int bw = b.width;
    int depth = 14 + (b.seed % 10);

    // side face (3D extrusion)
    Color sideCol = {18, 18, 24, 255};
    DrawTriangle(
      {(float)x + bw, (float)by},
      {(float)x + bw + depth, (float)by - depth * 0.4f},
      {(float)x + bw + depth, (float)by + bh - depth * 0.4f},
      sideCol);
    DrawTriangle(
      {(float)x + bw, (float)by},
      {(float)x + bw + depth, (float)by + bh - depth * 0.4f},
      {(float)x + bw, (float)by + bh},
      sideCol);

    // roof top edge
    Color roof = {30, 30, 38, 255};
    DrawTriangle(
      {(float)x, (float)by},
      {(float)x + bw, (float)by},
      {(float)x + bw + depth, (float)by - depth * 0.4f},
      roof);
    DrawTriangle(
      {(float)x, (float)by},
      {(float)x + bw + depth, (float)by - depth * 0.4f},
      {(float)x + depth * 0.5f, (float)by - depth * 0.4f},
      roof);

    // front face
    Color front = {28 + (b.seed % 12), 28, 36, 255};
    DrawRectangle((int)x, by, bw, bh, front);

    // windows grid with night glow
    bool night = timeOfDay_ < 7.f || timeOfDay_ > 20.f;
    int rows = 3 + (b.seed % 2);
    int cols = 2 + (b.seed % 3);
    int cellW = (bw - 12) / cols;
    int gapY = (bh - 24) / (rows + 1);
    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        int wx = (int)x + 6 + c * cellW;
        int wy = by + 14 + r * gapY;
        bool lit = night && ((b.seed + r * 5 + c * 3) % 4 != 0);
        Color wc = lit ? Color{255, 210, 120, 210} : Color{45, 50, 60, 255};
        DrawRectangle(wx, wy, cellW - 5, 11, wc);
        if (lit) DrawRectangle(wx, wy, cellW - 5, 2, Fade(WHITE, 0.25f));
      }
    }
  }
}

void World::DrawGround(int sw, int sh) {
  int gy = sh * 2 / 3;
  DrawRectangle(0, gy, sw, sh - gy, Color{32, 32, 38, 255});
  // perspective road
  DrawRectangle(0, gy + 6, sw, 4, Color{55, 55, 62, 255});
  for (int i = 0; i < 8; ++i) {
    float lx = fmodf(i * 160.f + scroll_ * 1.5f, sw + 80.f) - 40.f;
    DrawRectangle((int)lx, gy + 28, 40, 5, Color{200, 200, 180, 100});
  }
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
