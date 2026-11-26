#pragma once
#include "World/City.hpp"
#include "World/World.hpp"
#include "Character/Character.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Core/FontUI.hpp"
#include "raylib.h"
#include <string>
#include <cstdio>

class TravelScreen {
public:
  void Open() { open_ = true; sel_ = 0; msg_.clear(); }
  void Close() { open_ = false; }
  bool IsOpen() const { return open_; }

  void Update(Character& p, World& world) {
    if (!open_) return;
    const auto& cities = AllCities();
    int n = (int)cities.size();
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) sel_ = (sel_ + n - 1) % n;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) sel_ = (sel_ + 1) % n;
    if (IsKeyPressed(KEY_ENTER)) {
      const auto& c = cities[sel_];
      if (c.id == world.CurrentCityId()) {
        msg_ = "Ты уже здесь";
      } else {
        int64_t ticket = static_cast<int64_t>(8000 * c.priceMult) + 5000;
        if (p.Money() < ticket) {
          msg_ = std::string("Нужно ") + ArtworkRenderer::FormatRub(ticket);
        } else if (p.Energy() < 10) {
          msg_ = "Недостаточно энергии";
        } else {
          p.AddMoney(-ticket);
          p.SetEnergy(p.Energy() - 10);
          world.SetCity(c.id);
          p.SetCity(c.id);
          msg_ = std::string("Прибытие: ") + c.name;
        }
      }
    }
    if (IsKeyPressed(KEY_ESCAPE)) Close();
  }

  void Draw(int sw, int sh, World& world) {
    if (!open_) return;
    DrawRectangle(0, 0, sw, sh, Color{6, 8, 16, 240});
    FontUI::Get().Draw("ПЕРЕЕЗД / ВОКЗАЛ", 40, 28, 28, WHITE);
    FontUI::Get().Draw("A/D — город   ENTER — билет   ESC — назад", 40, 64, 16, Color{150, 150, 170, 255});
    const auto& cities = AllCities();
    const auto& c = cities[sel_ % cities.size()];
    char buf[128];
    snprintf(buf, sizeof(buf), "%s", c.name.c_str());
    FontUI::Get().Draw(buf, 40, 120, 26, Color{0, 220, 255, 255});
    FontUI::Get().Draw(c.desc.c_str(), 40, 160, 16, Color{180, 180, 200, 255});
    int64_t ticket = static_cast<int64_t>(8000 * c.priceMult) + 5000;
    snprintf(buf, sizeof(buf), "Билет %s   Риск %d   Множитель x%.2f",
             ArtworkRenderer::FormatRub(ticket).c_str(), c.danger, c.priceMult);
    FontUI::Get().Draw(buf, 40, 200, 18, Color{244, 196, 48, 255});
    // feature tags
    std::string tags;
    if (c.hasCasino) tags += "Казино ";
    if (c.hasClub) tags += "Клуб ";
    if (c.hasAirport) tags += "Аэропорт ";
    if (c.hasDriftTrack) tags += "Дрифт ";
    if (c.hasAuction) tags += "Аукцион ";
    if (c.hasStockExchange) tags += "Биржа ";
    if (!tags.empty()) FontUI::Get().Draw(tags.c_str(), 40, 240, 16, Color{160, 200, 180, 255});
    snprintf(buf, sizeof(buf), "Сейчас: %s", world.CurrentCityName().c_str());
    FontUI::Get().Draw(buf, 40, 280, 16, WHITE);
    if (!msg_.empty()) FontUI::Get().Draw(msg_.c_str(), 40, 320, 18, Color{100, 255, 140, 255});
  }

private:
  bool open_ = false;
  int sel_ = 0;
  std::string msg_;
};
