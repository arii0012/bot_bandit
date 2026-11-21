#include "CutsceneSystem.hpp"
#include "raylib.h"
#include <cmath>

void CutsceneSystem::Play(const std::string& id) {
  id_ = id;
  t_ = 0.f;
  active_ = true;
}

void CutsceneSystem::Update(float dt) {
  if (!active_) return;
  t_ += dt;
  float dur = 3.5f;
  if (id_ == "arrest") dur = 4.5f;
  if (id_ == "mansion") dur = 4.f;
  if (t_ > dur) active_ = false;
}

void CutsceneSystem::Draw(int sw, int sh) {
  if (!active_) return;
  int bar = 70 + (int)(sinf(t_ * 2.f) * 4);
  DrawRectangle(0, 0, sw, bar, BLACK);
  DrawRectangle(0, sh - bar, sw, bar, BLACK);

  const char* sub = id_.c_str();
  if (id_ == "debut") sub = "ДЕБЮТ — новый бандит в городе";
  else if (id_ == "keys") sub = "КЛЮЧИ ОТ МЕЧТЫ";
  else if (id_ == "vip") sub = "VIP — фейсконтроль пройден";
  else if (id_ == "travel") sub = "МЕЖДУГОРОДНИЙ РЕЙС";
  else if (id_ == "arrest") sub = "ОБЛАВА — slow-mo";
  else if (id_ == "mansion") sub = "НОВОСЕЛЬЕ";
  else if (id_ == "heist_win") sub = "УДАЧНЫЙ НАЛЁТ";

  int tw = MeasureText(sub, 24);
  DrawText(sub, (sw - tw) / 2, sh - bar + 22, 24, WHITE);

  // simple zoom vignette
  float pulse = 0.15f + 0.05f * sinf(t_ * 3.f);
  DrawRectangle(0, 0, sw, sh, Fade(BLACK, pulse * 0.3f));
}
