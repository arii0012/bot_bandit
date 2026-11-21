#include "CutsceneSystem.hpp"
#include "raylib.h"

void CutsceneSystem::Play(const std::string& id) {
  id_ = id;
  t_ = 0.f;
  active_ = true;
}

void CutsceneSystem::Update(float dt) {
  if (!active_) return;
  t_ += dt;
  if (t_ > 3.5f) active_ = false;
}

void CutsceneSystem::Draw(int sw, int sh) {
  if (!active_) return;
  // letterbox
  int bar = 70;
  DrawRectangle(0, 0, sw, bar, BLACK);
  DrawRectangle(0, sh - bar, sw, bar, BLACK);
  DrawText(id_.c_str(), 40, sh - bar + 25, 22, WHITE);
}
