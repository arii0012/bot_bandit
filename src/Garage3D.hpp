#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include "Procedural3DModeler.hpp"
#include "CarDatabase150.hpp"
#include "Character/Character.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>

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
    toast_.clear();
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
    int n = (int)cars_.size();
    if (n < 1) n = 1;
    if (IsKeyPressed(KEY_Q)) sel_ = (sel_ + n - 1) % n;
    if (IsKeyPressed(KEY_E)) sel_ = (sel_ + 1) % n;
    if (IsKeyPressed(KEY_ONE)) {
      p.neonCar_ = true;
      toast_ = "Neon ON";
    }
    if (IsKeyPressed(KEY_TWO)) {
      p.carR_ = (unsigned char)GetRandomValue(20, 255);
      p.carG_ = (unsigned char)GetRandomValue(20, 255);
      p.carB_ = (unsigned char)GetRandomValue(20, 255);
      toast_ = "Paint";
    }
    if (IsKeyPressed(KEY_THREE)) {
      p.engineLvl_ = std::min(5, p.engineLvl_ + 1);
      toast_ = "Chip +";
    }
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_G)) Close();
    wheel_ += dt * 90.f;
  }

  void Draw(Character& p) {
    if (!open_) return;
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    Camera3D cam{};
    cam.position = {sinf(camYaw_ * DEG2RAD) * camDist_, 2.2f, cosf(camYaw_ * DEG2RAD) * camDist_};
    cam.target = {0.f, 0.5f, 0.f};
    cam.up = {0.f, 1.f, 0.f};
    cam.fovy = 50.f;
    cam.projection = CAMERA_PERSPECTIVE;

    ClearBackground(Color{12, 12, 18, 255});
    BeginMode3D(cam);
    DrawPlane({0, 0, 0}, {20, 20}, Color{28, 28, 34, 255});
    DrawGrid(20, 1.f);
    DrawCube({0, 2, -8}, 16, 4, 0.3f, Color{40, 40, 48, 255});
    Color body = {p.carR_, p.carG_, p.carB_, 255};
    Procedural3DModeler::DrawCar3D(body, {0, 0, 0}, 0, wheel_, p.neonCar_, true, 1.2f);
    EndMode3D();

    float ph = 120.f;
    Rectangle panel{20.f, (float)sh - ph - 16.f, (float)sw - 40.f, ph};
    DrawRectangleRounded(panel, 0.08f, 10, Color{16, 16, 26, 230});
    DrawRectangleRoundedLines(panel, 0.08f, 10, Color{220, 40, 70, 180});

    std::string car = cars_.empty() ? "EMPTY - buy car in shop" : cars_[sel_ % cars_.size()];
    FontUI::Get().Draw(car.c_str(), panel.x + 24, panel.y + 14, 22, WHITE);

    char stats[128];
    snprintf(stats, sizeof(stats), "Engine L%d  Turbo L%d  Neon %s", p.engineLvl_, p.turboLvl_,
             p.neonCar_ ? "ON" : "OFF");
    FontUI::Get().Draw(stats, panel.x + 24, panel.y + 42, 16, Color{160, 160, 180, 255});

    auto btn = [&](float bx, const char* lab, float bw) -> bool {
      Rectangle r{panel.x + bx, panel.y + 70.f, bw, 36.f};
      bool h = CheckCollisionPointRec(GetMousePosition(), r);
      DrawRectangleRounded(r, 0.25f, 8, h ? Color{255, 60, 90, 255} : Color{40, 40, 58, 255});
      int tw = FontUI::Get().Measure(lab, 16);
      FontUI::Get().Draw(lab, r.x + (r.width - tw) * 0.5f, r.y + 8.f, 16, WHITE);
      return h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    };

    int n = (int)cars_.size();
    if (n < 1) n = 1;
    if (btn(20.f, "< PREV", 100.f)) sel_ = (sel_ + n - 1) % n;
    if (btn(130.f, "NEXT >", 100.f)) sel_ = (sel_ + 1) % n;
    if (btn(250.f, "NEON", 90.f)) {
      p.neonCar_ = !p.neonCar_;
      toast_ = p.neonCar_ ? "Neon ON" : "Neon OFF";
    }
    if (btn(350.f, "PAINT", 90.f)) {
      p.carR_ = (unsigned char)GetRandomValue(20, 255);
      p.carG_ = (unsigned char)GetRandomValue(20, 255);
      p.carB_ = (unsigned char)GetRandomValue(20, 255);
      toast_ = "Paint";
    }
    if (btn(450.f, "CHIP", 90.f)) {
      p.engineLvl_ = std::min(5, p.engineLvl_ + 1);
      toast_ = "Chip +";
    }
    if (btn(550.f, "DRIVE", 100.f)) open_ = false;
    if (btn(panel.width - 140.f, "BACK", 110.f)) open_ = false;

    if (!toast_.empty())
      FontUI::Get().Draw(toast_.c_str(), panel.x + 24, panel.y - 28, 18, Color{0, 220, 255, 255});
  }

private:
  bool open_ = false;
  std::vector<std::string> cars_;
  int sel_ = 0;
  float camYaw_ = 30.f;
  float camDist_ = 6.f;
  float wheel_ = 0.f;
  float t_ = 0.f;
  std::string toast_;
};
