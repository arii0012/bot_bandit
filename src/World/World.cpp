#include "World/World.hpp"
#include "raylib.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

static Color shade_body(Color c) {
  return Color{
    (unsigned char)(c.r * 7 / 10),
    (unsigned char)(c.g * 7 / 10),
    (unsigned char)(c.b * 7 / 10),
    c.a};
}

World::World() {
  EnsureBuildings(1280);
}

void World::EnsureBuildings(int sw) {
  if (sw == lastSw_ && !buildings_.empty()) return;
  lastSw_ = sw;
  buildings_.clear();
  int n = 18 + (sw / 160);
  for (int i = 0; i < n; ++i) {
    Building b;
    b.baseX = (float)i / (float)n + (GetRandomValue(0, 20) / 1000.f);
    b.width = 40 + GetRandomValue(0, 50);
    b.height = 80 + GetRandomValue(0, 220);
    b.seed = i * 17 + 3;
    buildings_.push_back(b);
  }
}

void World::Update(float dt) {
  if (dt > 0.1f) dt = 0.1f;
  timeOfDay_ = fmodf(timeOfDay_ + dt * 0.03f, 24.f);
  scroll_ += dt * 18.f;
  trafficT_ += dt;
  pedT_ += dt;
}

std::string World::CurrentCityName() const {
  const City* c = Current();
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
      skyTop_ = {6, 6, 18, 255};
      skyBot_ = {14, 10, 28, 255};
    } else if (t < 8.f || t > 19.f) {
      skyTop_ = {100, 48, 70, 255};
      skyBot_ = {180, 90, 60, 255};
    } else if (t < 10.f) {
      skyTop_ = {70, 120, 190, 255};
      skyBot_ = {180, 200, 230, 255};
    } else {
      skyTop_ = {45, 110, 200, 255};
      skyBot_ = {140, 180, 230, 255};
    }
  }
  DrawRectangleGradientV(0, 0, sw, sh, skyTop_, skyBot_);
  // stars at night
  if (t < 6.f || t > 20.f) {
    for (int i = 0; i < 40; ++i) {
      int sx = (i * 97 + (int)scroll_) % sw;
      int sy = (i * 53) % (sh / 2);
      DrawPixel(sx, sy, Color{200, 200, 230, 180});
    }
  }
}

void World::DrawBuildings(int sw, int sh) {
  EnsureBuildings(sw);
  bool night = timeOfDay_ < 6.5f || timeOfDay_ > 19.5f;
  float groundY = sh * 0.72f;

  for (auto& b : buildings_) {
    float x = fmodf(b.baseX * sw * 1.4f - scroll_ * 12.f, (float)sw + 120.f) - 60.f;
    float h = (float)b.height;
    float w = (float)b.width;
    float y = groundY - h;
    Color wall = {(unsigned char)(18 + (b.seed % 20)), (unsigned char)(20 + (b.seed % 15)),
                  (unsigned char)(28 + (b.seed % 25)), 255};
    DrawRectangle((int)x, (int)y, (int)w, (int)h, wall);
    // cornice
    DrawRectangle((int)x - 2, (int)y, (int)w + 4, 6, Color{40, 40, 50, 255});
    // windows
    int cols = std::max(2, (int)w / 14);
    int rows = std::max(2, (int)h / 18);
    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        int wx = (int)x + 4 + c * ((int)w / cols);
        int wy = (int)y + 10 + r * ((int)h / rows);
        int lit = ((b.seed + r * 3 + c * 7 + (int)timeOfDay_) % 5);
        Color win = night ? (lit < 3 ? Color{255, 210, 120, 220} : Color{20, 25, 40, 255})
                          : Color{120, 160, 200, 100};
        DrawRectangle(wx, wy, 8, 10, win);
      }
    }
    // neon sign on some buildings
    if (b.seed % 4 == 0) {
      const char* signs[] = {"CASINO", "CLUB", "AZS", "SHOP", "GARAGE"};
      const char* s = signs[b.seed % 5];
      Color neon = b.seed % 2 ? Color{255, 40, 90, 255} : Color{0, 220, 255, 255};
      DrawRectangle((int)x + 4, (int)y + 14, (int)w - 8, 16, Fade(neon, 0.25f));
      DrawText(s, (int)x + 8, (int)y + 16, 10, neon);
    }
  }
}

void World::DrawGround(int sw, int sh) {
  float groundY = sh * 0.72f;
  bool night = timeOfDay_ < 6.5f || timeOfDay_ > 19.5f;
  DrawRectangle(0, (int)groundY, sw, sh - (int)groundY, night ? Color{22, 22, 28, 255} : Color{40, 42, 48, 255});
  // road
  float roadY = groundY + 20.f;
  DrawRectangle(0, (int)roadY, sw, 48, Color{28, 28, 32, 255});
  // lane marks
  for (int i = 0; i < sw / 40 + 2; ++i) {
    float lx = fmodf(i * 40.f - scroll_ * 40.f, (float)sw + 40.f);
    DrawRectangle((int)lx, (int)roadY + 22, 20, 4, Color{200, 200, 120, 200});
  }
  // sidewalks
  DrawRectangle(0, (int)groundY, sw, 18, Color{48, 48, 56, 255});
  DrawRectangle(0, (int)roadY + 48, sw, 14, Color{48, 48, 56, 255});
  // crosswalk
  float cx = fmodf(sw * 0.4f - scroll_ * 10.f, (float)sw);
  for (int i = 0; i < 6; ++i)
    DrawRectangle((int)cx + i * 10, (int)roadY + 4, 6, 40, Color{220, 220, 230, 180});
}

void World::DrawTraffic(int sw, int sh) {
  float roadY = sh * 0.72f + 28.f;
  bool night = timeOfDay_ < 6.5f || timeOfDay_ > 19.5f;
  for (int i = 0; i < 6; ++i) {
    float speed = 40.f + (i % 3) * 25.f;
    float x = fmodf(i * 220.f + trafficT_ * speed, (float)sw + 100.f) - 50.f;
    Color body = {(unsigned char)(40 + i * 30), (unsigned char)(40 + (i * 50) % 120),
                  (unsigned char)(50 + i * 20), 255};
    DrawRectangleRounded(Rectangle{x, roadY, 54, 18}, 0.3f, 4, body);
    DrawRectangleRounded(Rectangle{x + 12, roadY - 10, 28, 12}, 0.2f, 4, shade_body(body));
    DrawCircle((int)(x + 12), (int)(roadY + 18), 5, BLACK);
    DrawCircle((int)(x + 42), (int)(roadY + 18), 5, BLACK);
    if (night) {
      DrawRectangle((int)(x + 50), (int)(roadY + 4), 6, 4, Color{255, 240, 180, 255});
      DrawRectangle((int)x - 2, (int)(roadY + 4), 4, 4, Color{220, 40, 40, 255});
    }
  }
}

void World::DrawPedestrians(int sw, int sh) {
  float walkY = sh * 0.72f + 6.f;
  for (int i = 0; i < 8; ++i) {
    float x = fmodf(i * 160.f + pedT_ * (20.f + i * 3.f) * ((i % 2) ? 1.f : -1.f), (float)sw + 40.f);
    if (x < 0) x += sw + 40.f;
    float bob = sinf(pedT_ * 8.f + i) * 2.f;
    Color skin = {210, 170, 140, 255};
    Color cloth = {(unsigned char)(40 + i * 20), (unsigned char)(40 + i * 10), 70, 255};
    DrawRectangle((int)x, (int)(walkY - 18 + bob), 8, 14, cloth);
    DrawCircle((int)(x + 4), (int)(walkY - 24 + bob), 5, skin);
    DrawRectangle((int)x, (int)(walkY - 4 + bob), 3, 10, Color{30, 30, 40, 255});
    DrawRectangle((int)x + 5, (int)(walkY - 4 + bob), 3, 10, Color{30, 30, 40, 255});
  }
}

void World::DrawStreetProps(int sw, int sh) {
  float groundY = sh * 0.72f;
  bool night = timeOfDay_ < 6.5f || timeOfDay_ > 19.5f;
  for (int i = 0; i < sw / 120 + 1; ++i) {
    float x = fmodf(i * 120.f - scroll_ * 8.f, (float)sw + 40.f);
    // lamp
    DrawRectangle((int)x, (int)groundY - 50, 4, 50, Color{60, 60, 70, 255});
    DrawCircle((int)x + 2, (int)groundY - 52, 6, night ? Color{255, 230, 150, 255} : Color{180, 180, 190, 255});
    if (night) {
      DrawTriangle(Vector2{(float)x + 2, groundY - 52}, Vector2{(float)x - 30, groundY},
                   Vector2{(float)x + 34, groundY}, Fade(Color{255, 220, 140, 255}, 0.12f));
    }
  }
}

void World::Draw(int sw, int sh) {
  DrawSky(sw, sh);
  DrawBuildings(sw, sh);
  DrawGround(sw, sh);
  DrawStreetProps(sw, sh);
  DrawTraffic(sw, sh);
  DrawPedestrians(sw, sh);

  // city label
  char buf[64];
  snprintf(buf, sizeof(buf), "%s  %.0f:00", CurrentCityName().c_str(), timeOfDay_);
  DrawRectangle(12, 12, 260, 28, Color{0, 0, 0, 140});
  DrawText(buf, 20, 18, 16, WHITE);
}
