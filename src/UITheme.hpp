#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include <cmath>
#include <functional>
#include <string>

namespace UITheme {

inline Color Bg() { return {8, 8, 14, 255}; }
inline Color Panel() { return {18, 18, 28, 220}; }
inline Color Panel2() { return {28, 28, 42, 235}; }
inline Color Accent() { return {220, 45, 75, 255}; }
inline Color Accent2() { return {0, 200, 255, 255}; }
inline Color Text() { return {245, 245, 250, 255}; }
inline Color Muted() { return {130, 130, 150, 255}; }
inline Color Gold() { return {244, 196, 48, 255}; }
inline Color Ok() { return {50, 200, 120, 255}; }

inline void GlassPanel(Rectangle r, float radius = 0.08f) {
  DrawRectangleRounded(r, radius, 12, Panel());
  DrawRectangleRoundedLines(r, radius, 12, Fade(Accent(), 0.55f));
  DrawRectangleRounded({r.x + 2, r.y + 2, r.width - 4, 18}, radius, 6, Fade(WHITE, 0.04f));
}

inline bool GlassButton(Rectangle r, const char* label, bool primary = false, float anim = 0.f) {
  Vector2 m = GetMousePosition();
  bool h = CheckCollisionPointRec(m, r);
  float pulse = 1.f + (h ? 0.03f * sinf(anim * 6.f) : 0.f);
  Rectangle rr = {r.x - (pulse - 1.f) * r.width * 0.5f, r.y - (pulse - 1.f) * r.height * 0.5f,
                  r.width * pulse, r.height * pulse};
  Color fill = primary ? (h ? Color{255, 70, 100, 255} : Accent()) : (h ? Color{50, 50, 70, 255} : Color{32, 32, 48, 255});
  DrawRectangleRounded(rr, 0.28f, 10, fill);
  DrawRectangleRoundedLines(rr, 0.28f, 10, h ? WHITE : Fade(Accent2(), 0.35f));
  if (h) DrawRectangleRounded(rr, 0.28f, 10, Fade(WHITE, 0.06f));
  float sz = 20.f;
  int tw = FontUI::Get().Ready() ? FontUI::Get().Measure(label, sz) : MeasureText(label, (int)sz);
  FontUI::Get().Draw(label, rr.x + (rr.width - tw) * 0.5f, rr.y + (rr.height - sz) * 0.5f, sz, Text());
  return h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

inline void Title(const char* t, float x, float y, float size = 36.f) {
  FontUI::Get().Draw(t, x + 2, y + 2, size, Fade(BLACK, 0.5f));
  FontUI::Get().Draw(t, x, y, size, Text());
}

inline void Caption(const char* t, float x, float y, float size = 16.f) {
  FontUI::Get().Draw(t, x, y, size, Muted());
}

} // namespace UITheme
