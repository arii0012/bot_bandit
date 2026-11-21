#include "World.hpp"
#include "Core/Config.hpp"
#include <cmath>

void World::Update(float dt) {
  anim_ += dt;
  timeOfDay_ += dt * 0.15f; // accelerated day cycle
  if (timeOfDay_ >= 24.f) timeOfDay_ -= 24.f;
}

std::string World::CurrentCityName() const {
  auto* c = FindCity(cityId_);
  return c ? c->name : cityId_;
}

const City* World::Current() const { return FindCity(cityId_); }

void World::SetCity(const std::string& id) {
  if (FindCity(id)) cityId_ = id;
}

void World::Draw(int sw, int sh) {
  // sky gradient by time
  float t = timeOfDay_;
  Color top, bot;
  if (t < 6 || t > 21) { // night
    top = {10, 10, 25, 255}; bot = {20, 15, 35, 255};
  } else if (t < 8 || t > 19) { // dawn/dusk
    top = {80, 40, 60, 255}; bot = {120, 60, 40, 255};
  } else {
    top = {40, 60, 90, 255}; bot = {70, 90, 110, 255};
  }
  DrawRectangleGradientV(0, 0, sw, sh / 2, top, bot);
  DrawRectangle(0, sh / 2, sw, sh / 2, bot);

  // ground
  DrawRectangle(0, sh * 2 / 3, sw, sh / 3, Color{30, 30, 35, 255});

  // simple building silhouettes
  for (int i = 0; i < 12; ++i) {
    int bx = (i * 140 + (int)(anim_ * 5)) % (sw + 100) - 50;
    int bh = 80 + (i * 37) % 160;
    int by = sh * 2 / 3 - bh;
    DrawRectangle(bx, by, 90, bh, Color{25, 25, 32, 255});
    // windows
    for (int wy = by + 10; wy < by + bh - 10; wy += 18)
      for (int wx = bx + 8; wx < bx + 80; wx += 16)
        DrawRectangle(wx, wy, 8, 10, (i + wy) % 5 == 0 ? Color{200, 180, 80, 180} : Color{40, 45, 55, 255});
  }

  // city name
  auto* c = Current();
  if (c) {
    DrawText(c->name.c_str(), 24, sh * 2 / 3 + 20, 28, Config::ColText());
    DrawText(c->desc.c_str(), 24, sh * 2 / 3 + 52, 18, Config::ColMuted());
  }
}
