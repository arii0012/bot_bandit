#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include "UITheme.hpp"
#include <string>
#include <vector>
#include <cmath>

struct StoryChapter {
  const char* id;
  const char* title;
  const char* objective;
  bool done;
};

struct CutKey {
  float t;
  const char* subtitle;
  float camYaw;
  float camH;
};

class StoryAndCutscenes {
public:
  StoryAndCutscenes() {
    chapters_ = {
      {"ch1", "Chapter 1: Streets", "Earn 50k and buy first car", false},
      {"ch2", "Chapter 2: Heat", "Survive 3 stars wanted", false},
      {"ch3", "Chapter 3: Club", "Own a club business", false},
      {"ch4", "Chapter 4: Empire", "Reach level 10", false},
      {"ch5", "Chapter 5: Legend", "Own mansion + rare car", false},
    };
  }

  void Play(const std::string& id) {
    active_ = true;
    t_ = 0.f;
    keys_.clear();
    if (id == "debut") {
      keys_ = {{0, "City does not forgive the weak...", 20, 2.5f},
               {3.5f, "But it remembers those who take risks.", 50, 3.f},
               {7.f, "Welcome to the Underworld.", 90, 2.2f}};
      duration_ = 10.f;
    } else if (id == "heist_win") {
      keys_ = {{0, "Clean job. Money is yours.", 10, 2.f}, {3.f, "Don't flash it too soon.", 40, 2.5f}};
      duration_ = 6.f;
    } else if (id == "arrest") {
      keys_ = {{0, "Hands where I can see them!", 0, 1.5f}, {2.5f, "Tonight the cell is your home.", 30, 2.f}};
      duration_ = 5.f;
    } else {
      keys_ = {{0, id.c_str(), 15, 2.f}};
      duration_ = 4.f;
    }
  }

  bool Active() const { return active_; }

  void Update(float dt) {
    if (!active_) return;
    t_ += dt;
    if (t_ >= duration_ || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE))
      active_ = false;
  }

  void Draw(int sw, int sh) {
    if (!active_) return;
    // letterbox
    int bar = sh / 8;
    DrawRectangle(0, 0, sw, bar, BLACK);
    DrawRectangle(0, sh - bar, sw, bar, BLACK);

    Camera3D cam{};
    const CutKey* k = &keys_[0];
    for (auto& x : keys_) if (t_ >= x.t) k = &x;
    cam.position = {sinf(k->camYaw * DEG2RAD) * 8.f, k->camH, cosf(k->camYaw * DEG2RAD) * 8.f};
    cam.target = {0, 1.f, 0};
    cam.up = {0, 1, 0};
    cam.fovy = 45.f;
    cam.projection = CAMERA_PERSPECTIVE;
    BeginMode3D(cam);
    DrawPlane({0, 0, 0}, {30, 30}, {20, 20, 28, 255});
    DrawCube({0, 1, 0}, 0.5f, 2.f, 0.5f, {80, 80, 100, 255});
    EndMode3D();

    if (k->subtitle) {
      int tw = FontUI::Get().Measure(k->subtitle, 22);
      FontUI::Get().Draw(k->subtitle, (sw - tw) * 0.5f, sh - bar + 20.f, 22, WHITE);
    }
    UITheme::Caption("Space — skip", 24, 24, 16);
  }

  std::vector<StoryChapter>& Chapters() { return chapters_; }
  void Complete(const std::string& id) {
    for (auto& c : chapters_) if (id == c.id) c.done = true;
  }

private:
  bool active_ = false;
  float t_ = 0.f, duration_ = 5.f;
  std::vector<CutKey> keys_;
  std::vector<StoryChapter> chapters_;
};
