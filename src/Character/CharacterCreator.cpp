#include "Character/CharacterCreator.hpp"
#include "Core/FontUI.hpp"
#include <cstdio>

void CharacterCreator::Open(Character& p) {
  open_ = true;
  confirmed_ = false;
  tab_ = 0;
  spin_ = p.Facing();
  (void)p;
}

void CharacterCreator::Update(float dt, Character& p) {
  if (!open_) return;
  spin_ += dt * 0.35f;
  p.SetFacing(spin_);
  p.Update(dt);

  if (IsKeyPressed(KEY_TAB)) tab_ = (tab_ + 1) % 4;
  if (IsKeyPressed(KEY_Q)) p.SetHairStyle((p.HairStyle() + 7) % 8);
  if (IsKeyPressed(KEY_E)) p.SetHairStyle((p.HairStyle() + 1) % 8);
  if (IsKeyPressed(KEY_ONE)) p.SetFemale(false);
  if (IsKeyPressed(KEY_TWO)) p.SetFemale(true);
  if (IsKeyPressed(KEY_ENTER)) {
    confirmed_ = true;
    open_ = false;
  }
}

void CharacterCreator::Draw(Character& p, int sw, int sh) {
  if (!open_) return;
  DrawRectangle(0, 0, sw, sh, Color{8, 8, 14, 255});

  float cx = sw * 0.58f, cy = sh * 0.52f;
  DrawEllipse((int)cx, (int)(cy + 95), 110, 28, Fade(Color{220, 40, 70, 255}, 0.3f));
  DrawEllipse((int)cx, (int)(cy + 95), 80, 18, Fade(Color{0, 200, 255, 255}, 0.2f));
  p.Draw(cx, cy);

  FontUI::Get().DrawNeon("CHARACTER STUDIO", 40, 28, 30, WHITE, Color{220, 40, 80, 120});
  FontUI::Get().Draw("1/2 gender   Q/E hair   TAB tabs   ENTER start", 40, 68, 16, Color{150, 150, 170, 255});

  float px = 36, py = 110;
  auto btn = [&](Rectangle r, const char* lab, bool on) -> bool {
    bool h = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRounded(r, 0.22f, 8, on || h ? Color{220, 45, 75, 255} : Color{32, 32, 48, 255});
    DrawRectangleRoundedLines(r, 0.22f, 8, Color{0, 200, 255, 100});
    int tw = FontUI::Get().Measure(lab, 16);
    FontUI::Get().Draw(lab, r.x + (r.width - tw) * 0.5f, r.y + 10, 16, WHITE);
    return h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
  };

  FontUI::Get().Draw("GENDER", px, py, 16, Color{140, 140, 160, 255});
  if (btn(Rectangle{px, py + 22, 100, 36}, "MALE", !p.GenderFemale())) p.SetFemale(false);
  if (btn(Rectangle{px + 110, py + 22, 100, 36}, "FEMALE", p.GenderFemale())) p.SetFemale(true);

  FontUI::Get().Draw("HAIR", px, py + 80, 16, Color{140, 140, 160, 255});
  if (btn(Rectangle{px, py + 102, 80, 34}, "PREV", false)) p.SetHairStyle((p.HairStyle() + 7) % 8);
  if (btn(Rectangle{px + 90, py + 102, 80, 34}, "NEXT", false)) p.SetHairStyle((p.HairStyle() + 1) % 8);

  Color cols[] = {{30, 25, 20, 255}, {220, 200, 120, 255}, {160, 40, 40, 255}, {40, 40, 50, 255}, {90, 60, 40, 255}, {200, 200, 210, 255}};
  for (int i = 0; i < 6; ++i) {
    Rectangle r{px + i * 42.f, py + 150, 36, 36};
    DrawRectangleRounded(r, 0.2f, 6, cols[i]);
    if (CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      p.SetHairColor(cols[i]);
  }

  FontUI::Get().Draw("PRESETS", px, py + 210, 16, Color{140, 140, 160, 255});
  if (btn(Rectangle{px, py + 232, 120, 34}, "Street", false)) {
    p.SetFemale(false); p.SetHairStyle(0); p.SetHairColor(cols[0]);
  }
  if (btn(Rectangle{px + 130, py + 232, 120, 34}, "VIP", false)) {
    p.SetFemale(false); p.SetHairStyle(1); p.SetHairColor(cols[3]);
  }
  if (btn(Rectangle{px, py + 274, 120, 34}, "Model", false)) {
    p.SetFemale(true); p.SetHairStyle(3); p.SetHairColor(cols[1]);
  }
  if (btn(Rectangle{px + 130, py + 274, 120, 34}, "Random", false)) {
    p.SetFemale(GetRandomValue(0, 1) == 1);
    p.SetHairStyle(GetRandomValue(0, 7));
    p.SetHairColor(cols[GetRandomValue(0, 5)]);
  }

  Rectangle start{(float)sw * 0.5f - 120.f, (float)sh - 80.f, 240.f, 50.f};
  if (btn(start, "START GAME", true)) {
    confirmed_ = true;
    open_ = false;
  }
}
