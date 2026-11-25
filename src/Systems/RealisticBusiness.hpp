#pragma once
#include "Character/Character.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Core/FontUI.hpp"
#include "raylib.h"
#include <string>
#include <vector>
#include <algorithm>

struct BusinessDef {
  const char* id;
  const char* name;
  int64_t price;
  int64_t baseIncomePerHour;
  int64_t upkeepPerHour; // salaries + tax + supply
  int racketRisk;       // 0..100
};

class RealisticBusiness {
public:
  static const std::vector<BusinessDef>& Catalog() {
    static const std::vector<BusinessDef> v = {
      {"biz_streetfood", "Tochka stritfuda", 3500000LL, 18000, 9000, 25},
      {"biz_carwash", "Avtomoika / Shinomontazh", 9500000LL, 42000, 22000, 30},
      {"biz_tuning", "Tyuning-atele", 24000000LL, 90000, 48000, 35},
      {"biz_club", "Nochnoy klub", 55000000LL, 180000, 110000, 45},
      {"biz_dealership", "Avtosalon", 140000000LL, 320000, 200000, 40},
      {"biz_casino", "VIP-Kazino", 320000000LL, 700000, 450000, 55},
      {"biz_holding", "Stroitelnyy holding", 750000000LL, 1200000, 800000, 50},
    };
    return v;
  }

  void Open() { open_ = true; sel_ = 0; msg_.clear(); }
  void Close() { open_ = false; }
  bool IsOpen() const { return open_; }

  std::string TryBuy(Character& p) {
    const auto& all = Catalog();
    if (sel_ < 0 || sel_ >= (int)all.size()) return "Bad";
    const auto& b = all[sel_];
    for (auto& o : owned_)
      if (o == b.id) return "Already owned";
    if (p.Money() < b.price) return std::string("Nuzhno ") + ArtworkRenderer::FormatRub(b.price);
    p.AddMoney(-b.price);
    owned_.push_back(b.id);
    return std::string("Kupleno: ") + b.name;
  }

  // call ~ once per game hour
  void TickHour(Character& p) {
    for (auto& id : owned_) {
      for (auto& b : Catalog()) {
        if (id != b.id) continue;
        int64_t net = b.baseIncomePerHour - b.upkeepPerHour;
        if (net > 0) p.AddMoney(net);
        else if (p.Money() >= -net) p.AddMoney(net);
        // racket roll
        if (GetRandomValue(0, 100) < b.racketRisk / 5) {
          int64_t fee = b.baseIncomePerHour / 4;
          if (p.Money() >= fee) p.AddMoney(-fee);
        }
      }
    }
  }

  void Update(Character& p) {
    if (!open_) return;
    int n = (int)Catalog().size();
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) sel_ = (sel_ + n - 1) % n;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) sel_ = (sel_ + 1) % n;
    if (IsKeyPressed(KEY_ENTER)) msg_ = TryBuy(p);
    if (IsKeyPressed(KEY_ESCAPE)) Close();
  }

  void Draw(int sw, int sh, Character& p) {
    if (!open_) return;
    DrawRectangle(0, 0, sw, sh, Color{8, 8, 16, 240});
    FontUI::Get().Draw("BIZNESY", 40, 28, 28, WHITE);
    FontUI::Get().Draw("A/D - vybor  ENTER - kupit  ESC - nazad", 40, 64, 16, Color{150, 150, 170, 255});
    const auto& b = Catalog()[sel_ % Catalog().size()];
    FontUI::Get().Draw(b.name, 40, 110, 24, WHITE);
    FontUI::Get().Draw(ArtworkRenderer::FormatRub(b.price).c_str(), 40, 150, 22, Color{244, 196, 48, 255});
    char buf[128];
    snprintf(buf, sizeof(buf), "Dohod/chas %s  Upkeep %s  Racket %d%%",
             ArtworkRenderer::FormatRub(b.baseIncomePerHour).c_str(),
             ArtworkRenderer::FormatRub(b.upkeepPerHour).c_str(), b.racketRisk);
    FontUI::Get().Draw(buf, 40, 190, 16, Color{180, 180, 200, 255});
    FontUI::Get().Draw((std::string("Balans ") + ArtworkRenderer::FormatRub(p.Money())).c_str(), 40, 230, 18,
                       Color{0, 220, 255, 255});
    if (!msg_.empty()) FontUI::Get().Draw(msg_.c_str(), 40, 270, 18, Color{100, 255, 140, 255});
  }

private:
  bool open_ = false;
  int sel_ = 0;
  std::string msg_;
  std::vector<std::string> owned_;
};
