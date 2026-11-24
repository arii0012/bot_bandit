#pragma once
#include "raylib.h"
#include "UITheme.hpp"
#include "CustomCursor.hpp"
#include "Core/FontUI.hpp"
#include <cmath>
#include <string>

enum class MainMenuAction {
  None, Continue, NewGame, CharacterStudio, Garage3D, Settings, Exit
};

class MainMenu {
public:
  void Open(bool hasSave) {
    visible_ = true;
    hasSave_ = hasSave;
    camAngle_ = 0.f;
    hover_ = -1;
  }
  void Close() { visible_ = false; }
  bool Visible() const { return visible_; }

  MainMenuAction Update(float dt, CustomCursor& cursor) {
    if (!visible_) return MainMenuAction::None;
    camAngle_ += dt * 0.35f;
    anim_ += dt;
    cursor.Update(dt);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    BuildLayout(sw, sh);

    hover_ = -1;
    Vector2 m = GetMousePosition();
    for (int i = 0; i < btnCount_; ++i) {
      if (CheckCollisionPointRec(m, btns_[i])) hover_ = i;
    }
    cursor.SetHover(hover_ >= 0);

    if (hover_ >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      return actions_[hover_];
    }
    return MainMenuAction::None;
  }

  void Draw() {
    if (!visible_) return;
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    BuildLayout(sw, sh);

    // night sky
    DrawRectangle(0, 0, sw, sh, Color{6, 6, 12, 255});
    for (int i = 0; i < 60; ++i) {
      float sx = fmodf(i * 97.3f + camAngle_ * 8.f, (float)sw);
      float sy = fmodf(i * 53.1f, sh * 0.45f);
      DrawPixel((int)sx, (int)sy, Color{180, 180, 220, 200});
    }

    // 3D-ish city parallax blocks
    float cx = sw * 0.58f, cy = sh * 0.62f;
    for (int layer = 0; layer < 3; ++layer) {
      float depth = 1.f + layer * 0.55f;
      for (int i = 0; i < 14; ++i) {
        float a = camAngle_ * (0.2f + layer * 0.08f) + i * 0.45f + layer;
        float d = (70.f + (i % 5) * 36.f) * depth;
        float x = cx + cosf(a) * d * 1.35f;
        float y = cy + sinf(a * 0.55f) * 18.f - layer * 12.f;
        float h = (50.f + (i * 41 + layer * 17) % 150) / depth;
        float w = (22.f + (i % 3) * 10.f) * (1.2f - layer * 0.15f);
        Color b = {(unsigned char)(12 + layer * 8 + i), (unsigned char)(14 + layer * 6),
                   (unsigned char)(28 + layer * 10 + i * 2), 255};
        DrawRectangle((int)(x - w * 0.5f), (int)(y - h), (int)w, (int)h, b);
        if ((i + layer) % 3 == 0) {
          DrawRectangle((int)(x - w * 0.2f), (int)(y - h + 10), 6, 6,
                        Color{255, 200, 80, (unsigned char)(120 + layer * 40)});
        }
      }
    }
    DrawEllipse((int)cx, (int)(cy + 36), 240, 28, Fade(BLACK, 0.4f));
    DrawCircleLines((int)cx, (int)(cy + 10), 100.f + sinf(anim_ * 2.f) * 4.f,
                    Color{0, 200, 255, 90});
    DrawCircleLines((int)cx, (int)(cy + 10), 130.f, Color{220, 40, 80, 70});

    // neon car silhouette
    float carX = cx, carY = cy - 10.f;
    DrawRectangle((int)(carX - 50), (int)(carY - 12), 100, 24, Color{30, 30, 40, 255});
    DrawRectangle((int)(carX - 30), (int)(carY - 28), 55, 18, Color{40, 40, 55, 255});
    DrawRectangle((int)(carX - 48), (int)(carY + 8), 18, 10, Color{20, 20, 25, 255});
    DrawRectangle((int)(carX + 30), (int)(carY + 8), 18, 10, Color{20, 20, 25, 255});
    DrawRectangle((int)(carX + 40), (int)(carY - 6), 8, 6, Color{255, 230, 150, 220});

    // title
    FontUI::Get().DrawNeon("BOT BANDIT", 56, 48, 52, Color{255, 255, 255, 255},
                           Color{220, 40, 80, 140});
    FontUI::Get().DrawShadow("UNDERWORLD", 60, 110, 26, Color{0, 220, 255, 255});
    FontUI::Get().Draw(u8"ГЛАВНОЕ МЕНЮ", 60, 148, 16, Color{140, 140, 160, 255});

    // buttons
    for (int i = 0; i < btnCount_; ++i) {
      Rectangle r = btns_[i];
      bool h = (i == hover_);
      Color fill = h ? Color{255, 60, 95, 255} : (i == 0 && hasSave_ ? Color{220, 45, 75, 255}
                                                                    : Color{32, 32, 48, 230});
      if (!primary_[i]) {
        fill = h ? Color{55, 55, 78, 255} : Color{28, 28, 42, 230};
      }
      DrawRectangleRounded(r, 0.28f, 10, fill);
      DrawRectangleRoundedLines(r, 0.28f, 10, h ? WHITE : Color{0, 200, 255, 90});
      int tw = FontUI::Get().Measure(labels_[i], 22);
      FontUI::Get().Draw(labels_[i], r.x + (r.width - tw) * 0.5f, r.y + (r.height - 22) * 0.5f, 22,
                         WHITE);
    }

    DrawRectangle(0, 0, sw, 28, Fade(BLACK, 0.35f));
    DrawRectangle(0, sh - 36, sw, 36, Fade(BLACK, 0.5f));
    FontUI::Get().Draw(u8"ЛКМ — выбор   ESC не закрывает игру", 24, (float)sh - 28, 14, Color{120, 120, 140, 255});
  }

private:
  void BuildLayout(int sw, int sh) {
    float bx = 56.f;
    float by = sh * 0.30f;
    float bw = 340.f, bh = 52.f, gap = 12.f;
    btnCount_ = 0;
    auto add = [&](const char* lab, MainMenuAction a, bool prim) {
      if (btnCount_ >= 8) return;
      labels_[btnCount_] = lab;
      actions_[btnCount_] = a;
      primary_[btnCount_] = prim;
      btns_[btnCount_] = Rectangle{bx, by + btnCount_ * (bh + gap), bw, bh};
      btnCount_++;
    };
    if (hasSave_) add("CONTINUE", MainMenuAction::Continue, true); // PRODOLZHIT
    add("NEW GAME", MainMenuAction::NewGame, !hasSave_);
    add("CHARACTER STUDIO", MainMenuAction::CharacterStudio, false);
    add("3D GARAGE", MainMenuAction::Garage3D, false);
    add("SETTINGS", MainMenuAction::Settings, false);
    add("EXIT", MainMenuAction::Exit, false);
    (void)sw;
  }

  bool visible_ = true;
  bool hasSave_ = false;
  float camAngle_ = 0.f;
  float anim_ = 0.f;
  int hover_ = -1;
  int btnCount_ = 0;
  Rectangle btns_[8]{};
  const char* labels_[8]{};
  MainMenuAction actions_[8]{};
  bool primary_[8]{};
};
