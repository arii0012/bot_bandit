#pragma once
#include "Character/Character.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Core/FontUI.hpp"
#include "raylib.h"
#include <string>
#include <cstdlib>

class ContainerAuction {
public:
  void Open() {
    open_ = true;
    bid_ = 50000 + GetRandomValue(0, 200000);
    prize_ = GetRandomValue(0, 4);
    msg_.clear();
  }
  void Close() { open_ = false; }
  bool IsOpen() const { return open_; }

  void Update(Character& p) {
    if (!open_) return;
    if (IsKeyPressed(KEY_ENTER)) {
      if (p.Money() < bid_) { msg_ = "Malo deneg"; return; }
      p.AddMoney(-bid_);
      if (prize_ == 0) { p.Inv().cars.push_back("priora"); msg_ = "Vyigral: Priora!"; }
      else if (prize_ == 1) { p.AddMoney(bid_ * 2); msg_ = "Vyigral: kesh"; }
      else if (prize_ == 2) { p.Inv().items.push_back("scrap"); msg_ = "Metallolom..."; }
      else { p.AddHeat(15); msg_ = "Goryachiy gruz - Heat+"; }
    }
    if (IsKeyPressed(KEY_ESCAPE)) Close();
  }

  void Draw(int sw, int sh) {
    if (!open_) return;
    DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 180});
    float x = sw * 0.5f - 220.f, y = sh * 0.5f - 100.f;
    DrawRectangleRounded(Rectangle{x, y, 440, 200}, 0.1f, 10, Color{20, 20, 32, 245});
    FontUI::Get().DrawCentered("SLEPOY KONTEYNER", sw * 0.5f, y + 20, 22, Color{244, 196, 48, 255});
    FontUI::Get().DrawCentered(ArtworkRenderer::FormatRub(bid_).c_str(), sw * 0.5f, y + 70, 20, WHITE);
    FontUI::Get().DrawCentered("ENTER - stavka  ESC - uytі", sw * 0.5f, y + 120, 14, Color{160, 160, 180, 255});
    if (!msg_.empty()) FontUI::Get().DrawCentered(msg_.c_str(), sw * 0.5f, y + 155, 16, Color{100, 255, 140, 255});
  }

private:
  bool open_ = false;
  int64_t bid_ = 0;
  int prize_ = 0;
  std::string msg_;
};
