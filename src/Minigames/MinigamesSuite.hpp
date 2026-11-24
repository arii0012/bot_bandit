#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>

enum class MiniGameId {
  None,
  PhoneHack,
  DJ,
  Lockpick,
  CasinoSlots
};

class MinigamesSuite {
public:
  void Start(MiniGameId id) {
    active_ = id;
    t_ = 0.f;
    score_ = 0;
    nodes_.clear();
    if (id == MiniGameId::PhoneHack) {
      for (int i = 0; i < 6; ++i)
        nodes_.push_back({(float)GetRandomValue(80, 500), (float)GetRandomValue(120, 320), false});
    }
    if (id == MiniGameId::DJ) beat_ = 0.f;
    if (id == MiniGameId::Lockpick) angle_ = 0.f, target_ = (float)GetRandomValue(20, 340);
    done_ = false;
    success_ = false;
  }
  void Close() { active_ = MiniGameId::None; }
  bool Active() const { return active_ != MiniGameId::None; }
  bool JustSucceeded() const { return done_ && success_; }
  bool JustFailed() const { return done_ && !success_; }
  MiniGameId Id() const { return active_; }

  void Update(float dt) {
    if (active_ == MiniGameId::None) return;
    t_ += dt;
    if (IsKeyPressed(KEY_ESCAPE)) { Close(); return; }

    if (active_ == MiniGameId::PhoneHack) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 m = GetMousePosition();
        for (auto& n : nodes_) {
          if (!n.hit && CheckCollisionPointCircle(m, Vector2{n.x, n.y}, 18.f)) {
            n.hit = true;
            score_++;
          }
        }
      }
      if (score_ >= (int)nodes_.size()) { done_ = true; success_ = true; active_ = MiniGameId::None; }
      if (t_ > 12.f) { done_ = true; success_ = false; active_ = MiniGameId::None; }
    } else if (active_ == MiniGameId::DJ) {
      beat_ += dt;
      if (IsKeyPressed(KEY_SPACE)) {
        float phase = fmodf(beat_ * 2.2f, 1.f);
        if (phase < 0.15f || phase > 0.85f) score_++;
        else score_ = std::max(0, score_ - 1);
      }
      if (score_ >= 12) { done_ = true; success_ = true; active_ = MiniGameId::None; }
      if (t_ > 30.f) { done_ = true; success_ = score_ >= 8; active_ = MiniGameId::None; }
    } else if (active_ == MiniGameId::Lockpick) {
      angle_ += dt * 40.f;
      if (IsKeyPressed(KEY_SPACE)) {
        float d = fabsf(fmodf(angle_, 360.f) - target_);
        if (d < 12.f || d > 348.f) { done_ = true; success_ = true; active_ = MiniGameId::None; }
        else score_++;
        if (score_ > 5) { done_ = true; success_ = false; active_ = MiniGameId::None; }
      }
    } else if (active_ == MiniGameId::CasinoSlots) {
      if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int a = GetRandomValue(0, 5), b = GetRandomValue(0, 5), c = GetRandomValue(0, 5);
        success_ = (a == b && b == c);
        score_ = success_ ? 3 : 0;
        done_ = true;
        active_ = MiniGameId::None;
      }
    }
  }

  void Draw(int sw, int sh) {
    if (active_ == MiniGameId::None) return;
    DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 180});
    float x = sw * 0.5f - 280.f, y = sh * 0.5f - 180.f;
    DrawRectangleRounded(Rectangle{x, y, 560, 360}, 0.08f, 12, Color{20, 20, 32, 240});
    DrawRectangleRoundedLines(Rectangle{x, y, 560, 360}, 0.08f, 12, Color{220, 40, 70, 200});

    if (active_ == MiniGameId::PhoneHack) {
      FontUI::Get().Draw("PHONE HACK - click nodes", x + 24, y + 20, 22, WHITE);
      for (auto& n : nodes_) {
        Color c = n.hit ? Color{40, 200, 90, 255} : Color{0, 200, 255, 255};
        DrawCircle(static_cast<int>(x + n.x), static_cast<int>(y + n.y), 16, c);
      }
      FontUI::Get().Draw(("Score " + std::to_string(score_)).c_str(), x + 24, y + 320, 18, WHITE);
    } else if (active_ == MiniGameId::DJ) {
      FontUI::Get().Draw("DJ DECK - SPACE on beat", x + 24, y + 20, 22, WHITE);
      float phase = fmodf(beat_ * 2.2f, 1.f);
      DrawRectangleRec(Rectangle{x + 40, y + 160, 480, 20}, Color{40, 40, 55, 255});
      DrawRectangleRec(Rectangle{x + 40 + phase * 480.f, y + 150, 12, 40}, Color{255, 60, 90, 255});
      FontUI::Get().Draw(("Hits " + std::to_string(score_)).c_str(), x + 24, y + 320, 18, WHITE);
    } else if (active_ == MiniGameId::Lockpick) {
      FontUI::Get().Draw("LOCKPICK - SPACE near target", x + 24, y + 20, 22, WHITE);
      Vector2 c{x + 280.f, y + 180.f};
      DrawCircleLines(static_cast<int>(c.x), static_cast<int>(c.y), 80, Color{0, 200, 255, 200});
      float rad = angle_ * DEG2RAD;
      DrawLineEx(c, Vector2{c.x + cosf(rad) * 70.f, c.y + sinf(rad) * 70.f}, 3.f, WHITE);
      float trad = target_ * DEG2RAD;
      DrawCircle(static_cast<int>(c.x + cosf(trad) * 80.f), static_cast<int>(c.y + sinf(trad) * 80.f), 8,
                 Color{255, 60, 90, 255});
    } else if (active_ == MiniGameId::CasinoSlots) {
      FontUI::Get().Draw("SLOTS - SPACE to spin", x + 24, y + 20, 22, WHITE);
      FontUI::Get().Draw("???  ???  ???", x + 180, y + 160, 32, Color{244, 196, 48, 255});
    }
    FontUI::Get().Draw("ESC - cancel", x + 24, y + 340, 14, Color{140, 140, 160, 255});
  }

private:
  struct Node { float x, y; bool hit; };
  MiniGameId active_ = MiniGameId::None;
  float t_ = 0.f, beat_ = 0.f, angle_ = 0.f, target_ = 0.f;
  int score_ = 0;
  bool done_ = false, success_ = false;
  std::vector<Node> nodes_;
};
