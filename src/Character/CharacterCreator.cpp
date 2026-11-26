#include "Character/CharacterCreator.hpp"
#include "Core/FontUI.hpp"
#include "raylib.h"
#include <string>
#include <cstdio>

void CharacterCreator::Open(Character* p) {
  player_ = p;
  open_ = true;
  tab_ = 0;
  spin_ = 0.f;
}

void CharacterCreator::Close() { open_ = false; }
bool CharacterCreator::IsOpen() const { return open_; }

static Color Palette(int i) {
  static Color cols[] = {
    {40, 30, 20, 255}, {20, 20, 25, 255}, {180, 40, 40, 255}, {40, 80, 180, 255},
    {40, 140, 60, 255}, {220, 180, 40, 255}, {200, 200, 210, 255}, {120, 40, 160, 255},
    {230, 120, 160, 255}, {90, 60, 40, 255}, {0, 200, 255, 255}, {255, 100, 40, 255}
  };
  return cols[i % 12];
}

void CharacterCreator::Update(float dt) {
  if (!open_ || !player_) return;
  spin_ += dt * 0.6f;
  if (IsKeyPressed(KEY_TAB)) tab_ = (tab_ + 1) % 8;
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) sel_ = (sel_ + 11) % 12;
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) sel_ = (sel_ + 1) % 12;
  if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
    Color c = Palette(sel_);
    switch (tab_) {
      case 0: player_->SetFemale(sel_ % 2 == 1); break;
      case 1: player_->colorEyes_ = c; break;
      case 2:
        player_->SetHairColor(c);
        player_->SetHairStyle(sel_ % 12);
        break;
      case 3: player_->colorTop_ = c; player_->topStyle_ = sel_ % 6; break;
      case 4: player_->colorPants_ = c; player_->pantsStyle_ = sel_ % 6; break;
      case 5: player_->colorHat_ = c; player_->hatStyle_ = sel_ % 6; break;
      case 6: player_->colorShoes_ = c; player_->shoeStyle_ = sel_ % 6; break;
      case 7: player_->colorAcc_ = c; player_->accStyle_ = sel_ % 6; break;
    }
  }
  if (IsKeyPressed(KEY_ESCAPE)) Close();
}

void CharacterCreator::Draw(int sw, int sh) {
  if (!open_ || !player_) return;
  DrawRectangle(0, 0, sw, sh, Color{10, 10, 18, 240});
  FontUI::Get().Draw("СТУДИЯ 360°", 40, 24, 28, WHITE);
  const char* tabs[] = {"Пол", "Глаза", "Волосы", "Торс", "Штаны", "Голова", "Обувь", "Аксесс"};
  for (int i = 0; i < 8; ++i) {
    Rectangle r{40.f + i * 100.f, 60.f, 94.f, 28.f};
    DrawRectangleRounded(r, 0.2f, 6, i == tab_ ? Color{255, 55, 95, 255} : Color{32, 32, 48, 255});
    FontUI::Get().DrawCentered(tabs[i], r.x + r.width * 0.5f, r.y + 6.f, 13, WHITE);
  }
  float cx = sw * 0.7f, cy = sh * 0.5f;
  player_->SetFacing(spin_);
  player_->Draw(cx, cy);
  for (int i = 0; i < 12; ++i) {
    Rectangle r{40.f + (i % 6) * 50.f, 120.f + (i / 6) * 50.f, 42.f, 42.f};
    DrawRectangleRec(r, Palette(i));
    if (i == sel_) DrawRectangleLinesEx(r, 2.f, WHITE);
  }
  FontUI::Get().Draw("TAB — вкладка   A/D — цвет   ENTER — применить   ESC — назад", 40, sh - 40.f, 14,
                     Color{140, 140, 160, 255});
  FontUI::Get().Draw(player_->GenderFemale() ? "Женская модель" : "Мужская модель", 40, 230, 16,
                     Color{0, 220, 255, 255});
}
