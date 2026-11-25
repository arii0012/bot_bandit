#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include "Character/Character.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include <cmath>
#include <cstdlib>
#include <string>
#include <algorithm>

enum class StoryMiniId { None, Delivery, DriftChase, CameraRoute, WorkShift };

class StoryMinigames {
public:
  void StartDelivery() {
    id_ = StoryMiniId::Delivery;
    t_ = 0; shake_ = 0; score_ = 0; done_ = false; success_ = false;
  }
  void StartDriftChase() {
    id_ = StoryMiniId::DriftChase;
    t_ = 0; score_ = 0; done_ = false; success_ = false; pos_ = 0.5f;
  }
  void StartCameraRoute() {
    id_ = StoryMiniId::CameraRoute;
    t_ = 0; score_ = 0; done_ = false; success_ = false; speed_ = 0.4f;
  }
  void StartWorkShift(const char* jobName, int seconds, int64_t pay) {
    id_ = StoryMiniId::WorkShift;
    job_ = jobName ? jobName : "Smena";
    t_ = 0; duration_ = static_cast<float>(seconds);
    pay_ = pay; done_ = false; success_ = false; taps_ = 0;
  }

  void Close() { id_ = StoryMiniId::None; }
  bool Active() const { return id_ != StoryMiniId::None; }
  bool JustFinished() const { return done_; }
  bool Success() const { return success_; }
  int64_t Pay() const { return pay_; }
  StoryMiniId Id() const { return id_; }

  void Update(float dt, Character& p) {
    if (id_ == StoryMiniId::None) return;
    if (dt > 0.1f) dt = 0.1f;
    t_ += dt;
    if (IsKeyPressed(KEY_ESCAPE)) { done_ = true; success_ = false; id_ = StoryMiniId::None; return; }

    if (id_ == StoryMiniId::Delivery) {
      // hold balance with A/D
      if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) shake_ -= dt * 1.2f;
      if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) shake_ += dt * 1.2f;
      shake_ += sinf(t_ * 3.f) * dt * 0.3f;
      if (fabsf(shake_) > 1.f) { done_ = true; success_ = false; id_ = StoryMiniId::None; return; }
      if (t_ >= 8.f) { done_ = true; success_ = true; p.AddMoney(8000); id_ = StoryMiniId::None; }
    } else if (id_ == StoryMiniId::DriftChase) {
      if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) pos_ -= dt * 0.7f;
      if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) pos_ += dt * 0.7f;
      pos_ = std::max(0.1f, std::min(0.9f, pos_));
      float obstacle = 0.5f + 0.3f * sinf(t_ * 2.2f);
      if (fabsf(pos_ - obstacle) < 0.08f) score_ -= 1;
      else score_ += 1;
      if (t_ >= 12.f) {
        done_ = true; success_ = score_ > 20;
        if (success_) p.AddMoney(25000);
        id_ = StoryMiniId::None;
      }
    } else if (id_ == StoryMiniId::CameraRoute) {
      if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) speed_ = std::min(1.f, speed_ + dt);
      if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) speed_ = std::max(0.15f, speed_ - dt);
      // cameras at intervals - overspeed = fail points
      float cam = fmodf(t_ * speed_, 1.f);
      if (cam > 0.45f && cam < 0.55f && speed_ > 0.7f) score_ -= 2;
      else score_ += 1;
      if (t_ >= 15.f) {
        done_ = true; success_ = score_ > 30;
        if (success_) p.AddMoney(40000);
        else p.AddHeat(10);
        id_ = StoryMiniId::None;
      }
    } else if (id_ == StoryMiniId::WorkShift) {
      if (IsKeyPressed(KEY_SPACE)) taps_++;
      if (t_ >= duration_) {
        done_ = true; success_ = true;
        int64_t bonus = taps_ * 50;
        p.AddMoney(pay_ + bonus);
        id_ = StoryMiniId::None;
      }
    }
  }

  void Draw(int sw, int sh) {
    if (id_ == StoryMiniId::None) return;
    DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 160});
    float x = sw * 0.5f - 280.f, y = sh * 0.5f - 140.f;
    DrawRectangleRounded(Rectangle{x, y, 560, 280}, 0.08f, 12, Color{16, 16, 28, 245});
    DrawRectangleRoundedLines(Rectangle{x, y, 560, 280}, 0.08f, 12, Color{220, 40, 70, 180});

    if (id_ == StoryMiniId::Delivery) {
      FontUI::Get().DrawCentered("DOSTAVKA - ne tryasi yashchik", sw * 0.5f, y + 20, 20, WHITE);
      DrawRectangleRec(Rectangle{x + 80, y + 120, 400, 16}, Color{40, 40, 55, 255});
      DrawCircle(static_cast<int>(x + 280 + shake_ * 160), static_cast<int>(y + 128), 12, Color{0, 220, 255, 255});
      FontUI::Get().DrawCentered("A/D balans", sw * 0.5f, y + 200, 16, Color{160, 160, 180, 255});
    } else if (id_ == StoryMiniId::DriftChase) {
      FontUI::Get().DrawCentered("DRIFT + POGONYA 90s", sw * 0.5f, y + 20, 20, WHITE);
      DrawRectangleRec(Rectangle{x + 40, y + 140, 480, 40}, Color{30, 30, 40, 255});
      float obstacle = 0.5f + 0.3f * sinf(t_ * 2.2f);
      DrawRectangleRec(Rectangle{x + 40 + obstacle * 460.f, y + 140, 20, 40}, Color{255, 60, 90, 255});
      DrawRectangleRec(Rectangle{x + 40 + pos_ * 460.f, y + 148, 24, 24}, Color{0, 220, 255, 255});
    } else if (id_ == StoryMiniId::CameraRoute) {
      FontUI::Get().DrawCentered("MARSHRUT - kamery skorosti", sw * 0.5f, y + 20, 20, WHITE);
      char buf[48];
      snprintf(buf, sizeof(buf), "Skorost %.0f%%  Score %d", speed_ * 100.f, score_);
      FontUI::Get().DrawCentered(buf, sw * 0.5f, y + 100, 18, WHITE);
      FontUI::Get().DrawCentered("W/S - gaz/tormoz", sw * 0.5f, y + 160, 16, Color{160, 160, 180, 255});
    } else if (id_ == StoryMiniId::WorkShift) {
      FontUI::Get().DrawCentered(job_.c_str(), sw * 0.5f, y + 20, 22, WHITE);
      float p = duration_ > 0 ? t_ / duration_ : 1.f;
      DrawRectangleRec(Rectangle{x + 40, y + 100, 480, 18}, Color{40, 40, 55, 255});
      DrawRectangleRec(Rectangle{x + 40, y + 100, 480 * p, 18}, Color{0, 200, 255, 255});
      char buf[64];
      snprintf(buf, sizeof(buf), "SPACE - usilie  |  %s", ArtworkRenderer::FormatRub(pay_).c_str());
      FontUI::Get().DrawCentered(buf, sw * 0.5f, y + 160, 16, Color{244, 196, 48, 255});
    }
  }

private:
  StoryMiniId id_ = StoryMiniId::None;
  float t_ = 0, duration_ = 10, shake_ = 0, pos_ = 0.5f, speed_ = 0.4f;
  int score_ = 0, taps_ = 0;
  int64_t pay_ = 0;
  bool done_ = false, success_ = false;
  std::string job_;
};
