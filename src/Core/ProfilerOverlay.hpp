#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include <cstdio>

class ProfilerOverlay {
public:
  void Toggle() { visible_ = !visible_; }
  bool Visible() const { return visible_; }

  void BeginFrame() { t0_ = GetTime(); }
  void EndFrame() {
    double t1 = GetTime();
    float ms = static_cast<float>((t1 - t0_) * 1000.0);
    // EMA
    frameMs_ = frameMs_ * 0.9f + ms * 0.1f;
  }

  void Draw(int sw) {
    if (!visible_) return;
    char buf[128];
    snprintf(buf, sizeof(buf), "FPS %d  Frame %.2f ms  DT %.3f",
             GetFPS(), frameMs_, GetFrameTime());
    DrawRectangle(8, 8, 320, 28, Color{0, 0, 0, 180});
    FontUI::Get().Draw(buf, 14, 14, 16, Color{100, 255, 140, 255});
    (void)sw;
  }

private:
  bool visible_ = false;
  double t0_ = 0.0;
  float frameMs_ = 16.f;
};
