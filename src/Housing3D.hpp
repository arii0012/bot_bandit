#pragma once
#include "Character/Character.hpp"
#include "ShopCatalog.hpp"
#include "Core/FontUI.hpp"
#include "raylib.h"
#include <string>
#include <vector>

struct HouseDef3D {
  const char* id;
  const char* name;
  int price;
  int tier;
  int garageSlots;
};

class Housing3D {
public:
  static const std::vector<HouseDef3D>& All() {
    static const std::vector<HouseDef3D> v = {
      {"house_room", "Koyka v hostele", 80000, 1, 0},
      {"house_apartment", "Odushka", 450000, 1, 1},
      {"house_center", "Kv visokogo classa", 2500000, 2, 2},
      {"house_mansion", "Osobnyak", 15000000, 3, 4},
      {"house_live0", "Panelka", 900000, 1, 1},
      {"house_live1", "Townhouse", 3500000, 2, 2},
      {"house_live2", "Stalinka", 2800000, 2, 1},
      {"house_live3", "Cottage", 6000000, 3, 3},
      {"house_live4", "Villa", 18000000, 3, 5},
      {"house_extra0", "Loft", 5000000, 2, 2},
      {"house_extra1", "Penthouse", 22000000, 3, 3},
      {"house_extra2", "Studio", 1200000, 1, 1},
      {"house_extra3", "Hi-Tech", 12000000, 3, 3},
      {"house_extra4", "Estate", 35000000, 4, 6},
    };
    return v;
  }

  void Open() { open_ = true; sel_ = 0; }
  void Close() { open_ = false; }
  bool IsOpen() const { return open_; }

  std::string TryBuy(Character& p) {
    const auto& all = All();
    if (sel_ < 0 || sel_ >= (int)all.size()) return "Bad house";
    const auto& h = all[sel_];
    for (auto& owned : p.Inv().houses)
      if (owned == h.id) return "Already owned";
    if (p.Money() < h.price) return "Need more money";
    p.AddMoney(-h.price);
    p.Inv().houses.push_back(h.id);
    return std::string("Bought ") + h.name;
  }

  void Update(Character& p) {
    if (!open_) return;
    int n = (int)All().size();
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) sel_ = (sel_ + n - 1) % n;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) sel_ = (sel_ + 1) % n;
    if (IsKeyPressed(KEY_ENTER)) lastMsg_ = TryBuy(p);
    if (IsKeyPressed(KEY_ESCAPE)) Close();
  }

  void Draw(int sw, int sh, Character& p) {
    if (!open_) return;
    DrawRectangle(0, 0, sw, sh, Color{8, 8, 16, 240});
    FontUI::Get().Draw("NEDVIZHIMOST / 3D HOUSES", 40, 30, 28, WHITE);
    FontUI::Get().Draw("A/D — выбор   ENTER — купить   ESC — назад", 40, 68, 16, Color{150, 150, 170, 255});
    const auto& all = All();
    const auto& h = all[sel_ % all.size()];
    // building preview blocks
    float cx = sw * 0.7f, cy = sh * 0.55f;
    float hh = 80.f + h.tier * 40.f;
    DrawRectangleRec(Rectangle{cx - 50, cy - hh, 100, hh}, Color{40, 42, 55, 255});
    for (int r = 0; r < h.tier * 3; ++r)
      for (int c = 0; c < 3; ++c)
        DrawRectangleRec(Rectangle{cx - 40 + c * 28.f, cy - hh + 12 + r * 22.f, 16, 14},
                         Color{255, 210, 120, 180});
    DrawEllipse(static_cast<int>(cx), static_cast<int>(cy + 10), 70, 16, Fade(BLACK, 0.35f));

    char buf[128];
    snprintf(buf, sizeof(buf), "%s", h.name);
    FontUI::Get().Draw(buf, 40, 120, 24, WHITE);
    snprintf(buf, sizeof(buf), "Price $%d   Tier %d   Garage slots %d", h.price, h.tier, h.garageSlots);
    FontUI::Get().Draw(buf, 40, 160, 18, Color{244, 196, 48, 255});
    snprintf(buf, sizeof(buf), "Money $%lld", (long long)p.Money());
    FontUI::Get().Draw(buf, 40, 200, 18, Color{0, 220, 255, 255});
    if (!lastMsg_.empty()) FontUI::Get().Draw(lastMsg_.c_str(), 40, 240, 18, Color{100, 255, 140, 255});
  }

private:
  bool open_ = false;
  int sel_ = 0;
  std::string lastMsg_;
};
