#pragma once
#include "raylib.h"
#include <cmath>
#include <vector>

class CustomCursor {
public:
  void Init() {
    HideCursor();
    trail_.assign(16, {0, 0});
  }

  void Update(float dt) {
    t_ += dt;
    Vector2 m = GetMousePosition();
    for (int i = (int)trail_.size() - 1; i > 0; --i) trail_[i] = trail_[i - 1];
    trail_[0] = m;
    hot_ = false;
  }

  void SetHover(bool h) { hot_ = h; }

  void Draw() {
    Vector2 m = GetMousePosition();
    for (int i = (int)trail_.size() - 1; i >= 0; --i) {
      float a = 1.f - (float)i / (float)trail_.size();
      float rad = 3.f + a * 4.f;
      Color c = hot_ ? Color{0, 220, 255, (unsigned char)(a * 90)}
                     : Color{220, 40, 80, (unsigned char)(a * 80)};
      DrawCircleV(trail_[i], rad, c);
    }
    float pulse = 10.f + sinf(t_ * 8.f) * 2.f + (hot_ ? 3.f : 0.f);
    DrawCircleLines((int)m.x, (int)m.y, pulse, Color{255, 255, 255, 200});
    DrawCircleLines((int)m.x, (int)m.y, pulse + 3.f, hot_ ? Color{0, 220, 255, 120} : Color{220, 40, 80, 100});
    DrawCircleV(m, 5.f, WHITE);
    DrawCircleV(m, 3.2f, hot_ ? Color{0, 220, 255, 255} : Color{220, 45, 75, 255});
    // glass cross
    DrawLineEx({m.x - 14, m.y}, {m.x - 7, m.y}, 1.5f, Fade(WHITE, 0.7f));
    DrawLineEx({m.x + 7, m.y}, {m.x + 14, m.y}, 1.5f, Fade(WHITE, 0.7f));
    DrawLineEx({m.x, m.y - 14}, {m.x, m.y - 7}, 1.5f, Fade(WHITE, 0.7f));
    DrawLineEx({m.x, m.y + 7}, {m.x, m.y + 14}, 1.5f, Fade(WHITE, 0.7f));
  }

private:
  std::vector<Vector2> trail_;
  float t_ = 0.f;
  bool hot_ = false;
};
