#pragma once
#include "raylib.h"
#include "Procedural3DModeler.hpp"
#include "CarDatabase150.hpp"
#include "Character/Character.hpp"
#include "UITheme.hpp"
#include <string>
#include <vector>
#include <cmath>

class Garage3D {
public:
  void Open(Character& p) {
    open_ = true;
    cars_.clear();
    for (auto& c : p.Inv().cars) cars_.push_back(c);
    if (cars_.empty()) cars_.push_back("demo");
    sel_ = 0;
    camYaw_ = 30.f;
    camDist_ = 6.f;
  }
  void Close() { open_ = false; }
  bool IsOpen() const { return open_; }

  void Update(float dt, Character& p) {
    if (!open_) return;
    t_ += dt;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) camYaw_ -= 60.f * dt;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) camYaw_ += 60.f * dt;
    if (IsKeyDown(KEY_W)) camDist_ = std::max(3.f, camDist_ - 2.f * dt);
    if (IsKeyDown(KEY_S)) camDist_ = std::min(12.f, camDist_ + 2.f * dt);
    if (IsKeyPressed(KEY_Q)) sel_ = (sel_ + (int)cars_.size() - 1) % (int)cars_.size();
    if (IsKeyPressed(KEY_E)) sel_ = (sel_ + 1) % (int)cars_.size();
    if (IsKeyPressed(KEY_ONE)) { p.neonCar_ = true; toast_ = "Neon ON"; }
    if (IsKeyPressed(KEY_TWO)) {
      p.carR_ = GetRandomValue(20, 255); p.carG_ = GetRandomValue(20, 255); p.carB_ = GetRandomValue(20, 255);
      toast_ = "Paint";
    }
    if (IsKeyPressed(KEY_THREE)) { p.engineLvl_ = std::min(5, p.engineLvl_ + 1); toast_ = "Chip +"; }
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_G)) Close();
    wheel_ += dt * 90.f;
  }

  void Draw(Character& p) {
    if (!open_) return;
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    Camera3D cam{};
    cam.position = {sinf(camYaw_ * DEG2RAD) * camDist_, 2.2f, cosf(camYaw_ * DEG2RAD) * camDist_};
    cam.target = {0, 0.5f, 0};
    cam.up = {0, 1, 0};
    cam.fovy = 50.f;
    cam.projection = CAMERA_PERSPECTIVE;

    ClearBackground({12, 12, 18, 255});
    BeginMode3D(cam);
    DrawPlane({0, 0, 0}, {20, 20}, {28, 28, 34, 255});
    DrawGrid(20, 1.f);
    // garage walls
    DrawCube({0, 2, -8}, 16, 4, 0.3f, {40, 40, 48, 255});
    Color body = {p.carR_, p.carG_, p.carB_, 255};
    Procedural3DModeler::DrawCar3D(body, {0, 0, 0}, 0, wheel_, p.neonCar_, true, 1.2f);
    EndMode3D();

    UITheme::GlassPanel({20, 20, 360, 200});
    UITheme::Title("3D GARAGE", 36, 32, 28);
    std::string car = cars_.empty() ? "-" : cars_[sel_ % cars_.size()];
    UITheme::Caption(("Auto: " + car).c_str(), 36, 70, 18);
    UITheme::Caption("A/D rotate  W/S zoom  Q/E switch", 36, 96, 16);
    UITheme::Caption("1 neon  2 paint  3 chip  Esc close", 36, 118, 16);
    if (!toast_.empty()) UITheme::Caption(toast_.c_str(), 36, 150, 18);
  }

private:
  bool open_ = false;
  std::vector<std::string> cars_;
  int sel_ = 0;
  float camYaw_ = 30.f, camDist_ = 6.f, wheel_ = 0.f, t_ = 0.f;
  std::string toast_;
};
