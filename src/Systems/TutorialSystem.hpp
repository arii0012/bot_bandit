#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include <string>
#include <vector>

struct TutorialStep {
  const char* title;
  const char* hint;
  int markerX, markerY;
};

class TutorialSystem {
public:
  void Start() {
    active_ = true;
    step_ = 0;
    skipped_ = false;
  }
  void Skip() { active_ = false; skipped_ = true; }
  void Restart() { Start(); }
  bool Active() const { return active_; }
  bool Skipped() const { return skipped_; }
  int Step() const { return step_; }

  void Next() {
    if (!active_) return;
    step_++;
    if (step_ >= (int)steps_.size()) active_ = false;
  }

  void Update(float dt) {
    if (!active_) return;
    anim_ += dt;
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) Next();
    if (IsKeyPressed(KEY_BACKSPACE)) Skip();
  }

  void Draw(int sw, int sh) {
    if (!active_ || step_ < 0 || step_ >= (int)steps_.size()) return;
    const auto& s = steps_[step_];
    // marker
    float pulse = 12.f + sinf(anim_ * 4.f) * 6.f;
    DrawCircle(s.markerX, s.markerY, pulse, Fade(Color{0, 220, 255, 255}, 0.25f));
    DrawCircleLines(s.markerX, s.markerY, static_cast<int>(pulse + 4), Color{0, 220, 255, 200});
    // banner
    float w = 520.f, h = 110.f;
    float x = (sw - w) * 0.5f, y = sh - h - 70.f;
    DrawRectangleRounded(Rectangle{x, y, w, h}, 0.1f, 10, Color{12, 12, 22, 230});
    DrawRectangleRoundedLines(Rectangle{x, y, w, h}, 0.1f, 10, Color{220, 40, 70, 200});
    char hdr[64];
    snprintf(hdr, sizeof(hdr), "ОБУЧЕНИЕ %d/%d", step_ + 1, (int)steps_.size());
    FontUI::Get().Draw(hdr, x + 20, y + 14, 16, Color{0, 220, 255, 255});
    FontUI::Get().Draw(s.title, x + 20, y + 40, 22, WHITE);
    FontUI::Get().Draw(s.hint, x + 20, y + 72, 16, Color{180, 180, 200, 255});
    FontUI::Get().Draw("SPACE — далее   BACKSPACE — пропуск", x + 20, y - 22, 14, Color{140, 140, 160, 255});
  }

private:
  bool active_ = false;
  bool skipped_ = false;
  int step_ = 0;
  float anim_ = 0.f;
  std::vector<TutorialStep> steps_ = {
    {"Осмотр", "WASD — движение, мышь — осмотр персонажа", 640, 360},
    {"Жилье", "Откройте смартфон (P) и купите первое жилье", 400, 500},
    {"Гараж", "Нажмите G — осмотрите 3D-гараж и авто", 900, 480},
    {"Смартфон", "P — банк, карта, DarkNet, такси", 700, 420},
    {"Заработок", "Работа, клуб или казино — первые деньги", 500, 400},
  };
};
