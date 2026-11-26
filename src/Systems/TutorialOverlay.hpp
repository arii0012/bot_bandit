#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include "UI/ActionPanel.hpp"
#include <cmath>
#include <cstdio>

class TutorialOverlay {
public:
  void Start() {
    active_ = true;
    step_ = 0;
    skipped_ = false;
  }
  void Skip() { active_ = false; skipped_ = true; highlight_ = ActionId::Count; }
  bool Active() const { return active_; }
  ActionId HighlightAction() const { return highlight_; }

  void Next() {
    if (!active_) return;
    step_++;
    if (step_ >= 5) {
      active_ = false;
      highlight_ = ActionId::Count;
    } else {
      SyncHighlight();
    }
  }

  void Update(float /*dt*/) {
    if (!active_) return;
    SyncHighlight();
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) Next();
    if (IsKeyPressed(KEY_BACKSPACE)) Skip();
  }

  void Draw(int sw, int sh) {
    if (!active_ || step_ < 0 || step_ >= 5) return;
    float pulse = 0.6f + 0.4f * sinf(static_cast<float>(GetTime()) * 4.f);
    // marker in alley
    int mx = sw / 2, my = sh / 2 - 40;
    DrawCircle(mx, my, 18.f + pulse * 8.f, Fade(Color{0, 220, 255, 255}, 0.25f));
    DrawCircleLines(mx, my, static_cast<int>(22 + pulse * 6), Color{0, 220, 255, 200});

    float w = 520.f, h = 120.f;
    float x = (sw - w) * 0.5f, y = 48.f;
    DrawRectangleRounded(Rectangle{x, y, w, h}, 0.1f, 10, Color{12, 12, 22, 235});
    DrawRectangleRoundedLines(Rectangle{x, y, w, h}, 0.1f, 10, Color{220, 40, 70, 200});

    char hdr[48];
    snprintf(hdr, sizeof(hdr), "ОБУЧЕНИЕ %d/5", step_ + 1);
    FontUI::Get().Draw(hdr, x + 18, y + 12, 14, Color{0, 220, 255, 255});
    FontUI::Get().Draw(titles_[step_], x + 18, y + 36, 20, WHITE);
    FontUI::Get().Draw(hints_[step_], x + 18, y + 68, 15, Color{180, 180, 200, 255});
    FontUI::Get().Draw("ПРОБЕЛ — далее   BACKSPACE — пропуск", x + 18, y + 96, 13, Color{130, 130, 150, 255});

    // skip button
    Rectangle skip{sw - 160.f, 16.f, 140.f, 32.f};
    bool hs = CheckCollisionPointRec(GetMousePosition(), skip);
    DrawRectangleRounded(skip, 0.25f, 8, hs ? Color{255, 60, 90, 255} : Color{36, 36, 52, 255});
    FontUI::Get().DrawCentered("ПРОПУСТИТЬ", skip.x + skip.width * 0.5f, skip.y + 8.f, 14, WHITE);
    if (hs && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) Skip();
  }

private:
  void SyncHighlight() {
    switch (step_) {
      case 0: highlight_ = ActionId::Work; break;
      case 1: highlight_ = ActionId::Rest; break;
      case 2: highlight_ = ActionId::Work; break;
      case 3: highlight_ = ActionId::Shop; break;
      case 4: highlight_ = ActionId::Fence; break;
      default: highlight_ = ActionId::Count; break;
    }
  }

  bool active_ = false;
  bool skipped_ = false;
  int step_ = 0;
  ActionId highlight_ = ActionId::Count;
  const char* titles_[5] = {
    "Энергия и смена",
    "Отдых",
    "Накопи на ВАЗ",
    "Магазин и телефон",
    "Барыга и сохранение",
  };
  const char* hints_[5] = {
    "Нажми РАБОТА - тратится ЭН, капают рубли",
    "Мало ЭН? ОТДЫХ ili dom vosst usuallyayut sily",
    "Без тачки ты пешком - копи на Приору",
    "МАГАЗИН: одежда, авто. P — смартфон",
    "БАРЫГА сбывает краденое. ESC — меню, сейв",
  };
};
