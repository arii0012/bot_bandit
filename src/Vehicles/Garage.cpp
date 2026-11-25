#include "Vehicles/Garage.hpp"
#include "Core/FontUI.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Systems/StoryDirector.hpp"
#include "raylib.h"
#include <algorithm>
#include <cstdio>

static int GarageStageFromAct(int act) {
  if (act <= 2) return 1;
  if (act <= 5) return 2;
  if (act <= 8) return 3;
  return 4;
}

void Garage::Open(Character& p) {
  open_ = true;
  cars_ = p.Inv().cars;
  sel_ = 0;
}

void Garage::Close() { open_ = false; }

bool Garage::IsOpen() const { return open_; }

void Garage::Update(Character& p) {
  if (!open_) return;
  cars_ = p.Inv().cars;
  int n = std::max(1, (int)cars_.size());
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) sel_ = (sel_ + n - 1) % n;
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) sel_ = (sel_ + 1) % n;
  if (IsKeyPressed(KEY_ESCAPE)) Close();
}

void Garage::Draw(int sw, int sh, Character& p) {
  if (!open_) return;
  int act = StoryDirector::Get().Act();
  int stage = GarageStageFromAct(act > 0 ? act : 1);
  cars_ = p.Inv().cars;

  // room background by stage
  Color wall, floor, accent;
  if (stage == 1) {
    wall = Color{70, 55, 45, 255}; floor = Color{45, 40, 35, 255}; accent = Color{180, 140, 40, 200};
  } else if (stage == 2) {
    wall = Color{90, 90, 85, 255}; floor = Color{55, 55, 50, 255}; accent = Color{100, 160, 220, 200};
  } else if (stage == 3) {
    wall = Color{40, 42, 50, 255}; floor = Color{30, 32, 38, 255}; accent = Color{0, 200, 255, 200};
  } else {
    wall = Color{18, 18, 24, 255}; floor = Color{28, 28, 36, 255}; accent = Color{220, 40, 80, 200};
  }

  DrawRectangle(0, 0, sw, sh, wall);
  DrawRectangle(0, sh * 2 / 3, sw, sh / 3, floor);

  // stage props
  if (stage == 1) {
    DrawRectangleRec(Rectangle{40, 80, 12, (float)sh * 0.55f}, Color{40, 30, 25, 255}); // pillar
    DrawCircle(sw / 2, 50, 18, Color{200, 180, 80, 180}); // bulb
    DrawLine(sw / 2, 0, sw / 2, 50, Color{60, 60, 50, 255});
    FontUI::Get().Draw("Kooperativ", 40, 30, 18, Color{200, 180, 140, 255});
  } else if (stage == 2) {
    DrawRectangleRec(Rectangle{sw - 120.f, 100.f, 60, 80}, Color{60, 60, 70, 255}); // compressor
    DrawRectangleRec(Rectangle{50, 120, 40, 100}, Color{70, 70, 80, 255}); // rack
    FontUI::Get().Draw("Boks", 40, 30, 18, Color{200, 200, 180, 255});
  } else if (stage == 3) {
    // lift
    DrawRectangleRec(Rectangle{(float)sw * 0.35f, (float)sh * 0.45f, (float)sw * 0.3f, 12}, Color{80, 80, 90, 255});
    DrawRectangleRec(Rectangle{(float)sw * 0.35f, (float)sh * 0.45f - 80, 10, 80}, Color{100, 100, 110, 255});
    DrawRectangleRec(Rectangle{(float)sw * 0.65f - 10, (float)sh * 0.45f - 80, 10, 80}, Color{100, 100, 110, 255});
    DrawRectangleRec(Rectangle{30, 100, 8, 200}, Fade(accent, 0.5f)); // neon
    FontUI::Get().Draw("Tyuning-atele", 40, 30, 18, accent);
  } else {
    // showroom floor reflection strip
    DrawRectangleRec(Rectangle{0, (float)sh * 0.66f, (float)sw, 4}, Fade(WHITE, 0.15f));
    DrawRectangleRec(Rectangle{(float)sw * 0.2f, (float)sh * 0.55f, (float)sw * 0.6f, 6}, Fade(accent, 0.4f));
    FontUI::Get().Draw("Showroom / Bunker", 40, 30, 18, accent);
  }

  FontUI::Get().Draw("GARAZH", 40, 55, 28, WHITE);
  char stbuf[48];
  snprintf(stbuf, sizeof(stbuf), "Stadiya %d/4  Akt %d", stage, act);
  FontUI::Get().Draw(stbuf, 40, 90, 16, Color{160, 160, 180, 255});

  if (cars_.empty()) {
    // empty garage - only pit and tools
    float cx = sw * 0.5f, cy = sh * 0.55f;
    DrawRectangleRec(Rectangle{cx - 80, cy, 160, 20}, Color{25, 25, 30, 255}); // pit
    DrawRectangleRec(Rectangle{cx - 100, cy - 40, 30, 50}, Color{50, 45, 40, 255}); // jack
    FontUI::Get().DrawCentered("Pusto. Kupi VAZ/Prioru v magazine.", sw * 0.5f, cy + 50, 18, Color{180, 180, 200, 255});
    FontUI::Get().DrawCentered("ESC - nazad", sw * 0.5f, sh - 40.f, 14, Color{140, 140, 160, 255});
    return;
  }

  // car slots on podium
  int n = (int)cars_.size();
  for (int i = 0; i < n && i < 6; ++i) {
    float px = 120.f + i * 180.f;
    float py = sh * 0.55f;
    bool sel = (i == sel_ % n);
    DrawEllipse((int)px, (int)(py + 30), 50, 12, Fade(BLACK, 0.35f));
    if (sel) DrawEllipse((int)px, (int)(py + 30), 55, 14, Fade(accent, 0.35f));
    // simple car body
    Color body = sel ? Color{200, 40, 50, 255} : Color{40, 50, 80, 255};
    DrawRectangleRec(Rectangle{px - 40, py - 20, 80, 28}, body);
    DrawRectangleRec(Rectangle{px - 25, py - 35, 45, 18}, Fade(body, 0.8f));
    DrawCircle((int)(px - 25), (int)(py + 10), 10, Color{20, 20, 25, 255});
    DrawCircle((int)(px + 25), (int)(py + 10), 10, Color{20, 20, 25, 255});
    FontUI::Get().DrawCentered(cars_[i].c_str(), px, py + 45, 13, sel ? WHITE : Color{160, 160, 180, 255});
  }

  std::string cur = cars_[sel_ % n];
  FontUI::Get().Draw((std::string("Vybrano: ") + cur).c_str(), 40, sh - 70.f, 18, Color{244, 196, 48, 255});
  FontUI::Get().Draw("A/D - vybor  ESC - nazad", 40, sh - 42.f, 14, Color{140, 140, 160, 255});
}
