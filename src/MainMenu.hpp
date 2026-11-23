#pragma once
#include "raylib.h"
#include "UITheme.hpp"
#include "CustomCursor.hpp"
#include "Core/FontUI.hpp"
#include <cmath>
#include <string>

enum class MainMenuAction {
  None,
  Continue,
  NewGame,
  CharacterStudio,
  Garage3D,
  Settings,
  Exit
};

class MainMenu {
public:
  void Open(bool hasSave) { visible_ = true; hasSave_ = hasSave; camAngle_ = 0.f; }
  void Close() { visible_ = false; }
  bool Visible() const { return visible_; }

  MainMenuAction Update(float dt, CustomCursor& cursor) {
    if (!visible_) return MainMenuAction::None;
    camAngle_ += dt * 0.25f;
    anim_ += dt;
    cursor.Update(dt);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    float bx = 64.f;
    float by = sh * 0.28f;
    float bw = 320.f, bh = 48.f, gap = 14.f;

    auto hit = [&](int i) {
      Rectangle r = {bx, by + i * (bh + gap), bw, bh};
      return CheckCollisionPointRec(GetMousePosition(), r);
    };
    bool any = false;
    for (int i = 0; i < 6; ++i) if (hit(i)) { any = true; break; }
    cursor.SetHover(any);

    int idx = 0;
    if (hasSave_ && UITheme::GlassButton({bx, by + idx * (bh + gap), bw, bh}, "Continue", true, anim_))
      return MainMenuAction::Continue;
    if (hasSave_) idx++;
    if (UITheme::GlassButton({bx, by + idx++ * (bh + gap), bw, bh}, "New Game", !hasSave_, anim_))
      return MainMenuAction::NewGame;
    if (UITheme::GlassButton({bx, by + idx++ * (bh + gap), bw, bh}, "Character Studio", false, anim_))
      return MainMenuAction::CharacterStudio;
    if (UITheme::GlassButton({bx, by + idx++ * (bh + gap), bw, bh}, "3D Garage", false, anim_))
      return MainMenuAction::Garage3D;
    if (UITheme::GlassButton({bx, by + idx++ * (bh + gap), bw, bh}, "Settings", false, anim_))
      return MainMenuAction::Settings;
    if (UITheme::GlassButton({bx, by + idx++ * (bh + gap), bw, bh}, "Exit", false, anim_))
      return MainMenuAction::Exit;
    return MainMenuAction::None;
  }

  void Draw() {
    if (!visible_) return;
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    // faux 3D city orbit background
    DrawRectangle(0, 0, sw, sh, UITheme::Bg());
    float cx = sw * 0.62f, cy = sh * 0.55f;
    for (int i = 0; i < 18; ++i) {
      float a = camAngle_ + i * 0.35f;
      float d = 80.f + (i % 5) * 40.f;
      float x = cx + cosf(a) * d * 1.4f;
      float y = cy + sinf(a * 0.7f) * 30.f - (i % 4) * 20.f;
      float h = 60.f + (i * 37) % 140;
      Color b = {(unsigned char)(20 + i * 3), 22, (unsigned char)(30 + i * 2), 255};
      DrawRectangle((int)x, (int)(y - h), 28 + (i % 3) * 10, (int)h, b);
      if ((i + (int)(camAngle_ * 3)) % 4 == 0)
        DrawRectangle((int)x + 6, (int)(y - h + 12), 8, 8, Fade(UITheme::Gold(), 0.5f));
    }
    DrawEllipse((int)cx, (int)(cy + 40), 220, 28, Fade(BLACK, 0.35f));
    // neon ground ring
    DrawCircleLines((int)cx, (int)(cy + 20), 90 + sinf(anim_ * 2.f) * 4.f, Fade(UITheme::Accent2(), 0.4f));
    DrawCircleLines((int)cx, (int)(cy + 20), 120, Fade(UITheme::Accent(), 0.25f));

    UITheme::Title("BOT BANDIT", 64, 48, 42);
    UITheme::Caption("UNDERWORLD", 68, 98, 20);
    UITheme::Caption("Main Menu", 68, 128, 16);

    // soft vignette
    DrawRectangle(0, 0, sw, 40, Fade(BLACK, 0.35f));
    DrawRectangle(0, sh - 40, sw, 40, Fade(BLACK, 0.45f));
  }

private:
  bool visible_ = true;
  bool hasSave_ = false;
  float camAngle_ = 0.f;
  float anim_ = 0.f;
};
