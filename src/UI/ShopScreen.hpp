#pragma once
#include "Character/Character.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Core/FontUI.hpp"
#include "Vehicles/VehicleDeepSystem.hpp"
#include "raylib.h"
#include <string>
#include <vector>
#include <cstdio>

struct ShopItem {
  const char* id;
  const char* name;
  const char* tab; // clothes cars houses weapons consumables
  int64_t price;
};

class ShopScreen {
public:
  void Open() { open_ = true; sel_ = 0; tab_ = 0; msg_.clear(); }
  void Close() { open_ = false; }
  bool IsOpen() const { return open_; }

  void Update(Character& p, VehicleDeepSystem* veh) {
    if (!open_) return;
    if (IsKeyPressed(KEY_TAB)) tab_ = (tab_ + 1) % 5;
    const auto items = Filtered();
    int n = (int)items.size();
    if (n <= 0) return;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) sel_ = (sel_ + n - 1) % n;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) sel_ = (sel_ + 1) % n;
    if (IsKeyPressed(KEY_ENTER)) {
      const auto& it = items[sel_ % n];
      if (p.Money() < it.price) {
        msg_ = std::string("Nuzhno ") + ArtworkRenderer::FormatRub(it.price);
      } else {
        p.AddMoney(-it.price);
        std::string tab = it.tab;
        if (tab == "clothes") p.Inv().clothes.push_back(it.id);
        else if (tab == "cars") {
          p.Inv().cars.push_back(it.id);
          p.SetActiveCar(it.id);
          if (veh) veh->OnCarBought();
        } else if (tab == "houses") p.Inv().houses.push_back(it.id);
        else if (tab == "weapons") p.Inv().weapons.push_back(it.id);
        else p.Inv().items.push_back(it.id);
        msg_ = std::string("Kupleno: ") + it.name;
      }
    }
    if (IsKeyPressed(KEY_ESCAPE)) Close();
  }

  void Draw(int sw, int sh, Character& p) {
    if (!open_) return;
    DrawRectangle(0, 0, sw, sh, Color{8, 8, 16, 245});
    FontUI::Get().Draw("MAGAZIN", 40, 24, 28, WHITE);
    const char* tabs[] = {"Odezhda", "Avto", "Dom", "Oruzhie", "Rashodka"};
    for (int i = 0; i < 5; ++i) {
      Rectangle r{40.f + i * 120.f, 60.f, 110.f, 32.f};
      DrawRectangleRounded(r, 0.2f, 6, i == tab_ ? Color{255, 55, 95, 255} : Color{32, 32, 48, 255});
      FontUI::Get().DrawCentered(tabs[i], r.x + r.width * 0.5f, r.y + 8.f, 14, WHITE);
    }
    FontUI::Get().Draw((std::string("Balans ") + ArtworkRenderer::FormatRub(p.Money())).c_str(), 40, 105, 18,
                       Color{244, 196, 48, 255});
    auto items = Filtered();
    for (int i = 0; i < (int)items.size() && i < 12; ++i) {
      Color c = (i == sel_) ? Color{0, 220, 255, 255} : WHITE;
      std::string line = std::string(items[i].name) + "  " + ArtworkRenderer::FormatRub(items[i].price);
      FontUI::Get().Draw(line.c_str(), 40, 140.f + i * 28.f, 16, c);
    }
    if (!msg_.empty()) FontUI::Get().Draw(msg_.c_str(), 40, sh - 80.f, 18, Color{100, 255, 140, 255});
    FontUI::Get().Draw("TAB - vkladka  W/S - vybor  ENTER - kupit  ESC - nazad", 40, sh - 48.f, 14,
                       Color{140, 140, 160, 255});
  }

private:
  std::vector<ShopItem> Filtered() const {
    static const ShopItem all[] = {
      {"hoodie", "Hudi ulits", "clothes", 25000},
      {"jacket", "Kozhanaya kurtka", "clothes", 85000},
      {"suit", "Kostyum VIP", "clothes", 180000},
      {"glasses", "Ochki", "clothes", 15000},
      {"chain", "Tsep' gold", "clothes", 120000},
      {"vaz2107", "VAZ 2107", "cars", 280000},
      {"priora", "Lada Priora", "cars", 550000},
      {"bmw3", "BMW 3", "cars", 4500000},
      {"lambo", "Lamborghini", "cars", 25000000},
      {"house_room", "Koyka", "houses", 80000},
      {"house_apartment", "Odushka", "houses", 450000},
      {"house_mansion", "Osobnyak", "houses", 15000000},
      {"knife", "Nozh", "weapons", 5000},
      {"pistol", "Pistolet", "weapons", 45000},
      {"energy", "Energetik", "consumables", 1500},
      {"insurance", "OSAGO", "consumables", 45000},
    };
    const char* tabNames[] = {"clothes", "cars", "houses", "weapons", "consumables"};
    std::vector<ShopItem> out;
    for (auto& it : all)
      if (std::string(it.tab) == tabNames[tab_ % 5]) out.push_back(it);
    return out;
  }

  bool open_ = false;
  int sel_ = 0, tab_ = 0;
  std::string msg_;
};
