#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include <cmath>
#include <string>

struct SpringF {
  float value = 0.f;
  float vel = 0.f;
  void To(float target, float dt, float stiff = 180.f, float damp = 18.f) {
    float force = (target - value) * stiff;
    vel += force * dt;
    vel *= (1.f - damp * dt);
    if (vel > 80.f) vel = 80.f;
    if (vel < -80.f) vel = -80.f;
    value += vel * dt;
  }
};

class LiquidGlassUI {
public:
  static void Panel(Rectangle r, float radius = 0.12f) {
    DrawRectangleRounded(r, radius, 14, Color{18, 18, 30, 180});
    DrawRectangleRounded(Rectangle{r.x + 2, r.y + 2, r.width - 4, r.height * 0.35f}, radius, 10,
                         Fade(WHITE, 0.05f));
    DrawRectangleRoundedLines(r, radius, 14, Color{220, 40, 80, 140});
  }

  static bool Button(Rectangle r, const char* label, float* hoverAnim) {
    Vector2 m = GetMousePosition();
    bool h = CheckCollisionPointRec(m, r);
    float target = h ? 1.f : 0.f;
    if (hoverAnim) {
      *hoverAnim += ((target - *hoverAnim) * 10.f) * GetFrameTime();
      r.x += (*hoverAnim) * 8.f;
      r.width += (*hoverAnim) * 4.f;
    }
    Color fill = h ? Color{255, 55, 95, 255} : Color{32, 32, 48, 210};
    DrawRectangleRounded(r, 0.28f, 12, fill);
    // cursor-following highlight
    if (h) {
      float hx = m.x - r.x;
      DrawRectangleRounded(Rectangle{r.x + hx - 30.f, r.y, 60.f, r.height}, 0.5f, 8,
                           Fade(WHITE, 0.08f));
    }
    DrawRectangleRoundedLines(r, 0.28f, 12, h ? WHITE : Color{0, 200, 255, 100});
    int tw = FontUI::Get().Measure(label, 18);
    FontUI::Get().Draw(label, r.x + (r.width - tw) * 0.5f, r.y + (r.height - 18) * 0.5f, 18, WHITE);
    return h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
  }

  static void DynamicIsland(int sw, const char* text, float pulse) {
    float w = 220.f + FontUI::Get().Measure(text, 14);
    if (w > sw - 40.f) w = sw - 40.f;
    float h = 34.f + sinf(pulse) * 2.f;
    float x = (sw - w) * 0.5f;
    float y = 12.f;
    DrawRectangleRounded(Rectangle{x, y, w, h}, 0.5f, 12, Color{10, 10, 16, 230});
    DrawRectangleRoundedLines(Rectangle{x, y, w, h}, 0.5f, 12, Color{0, 220, 255, 120});
    FontUI::Get().DrawCentered(text, sw * 0.5f, y + 8.f, 14, WHITE);
  }
};
