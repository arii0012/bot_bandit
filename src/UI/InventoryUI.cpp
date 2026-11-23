#include "InventoryUI.hpp"
#include <algorithm>
#include "Core/Application.hpp"
#include "UI/UISystem.hpp"
#include "Core/Config.hpp"
#include "Character/Character.hpp"
#include "Systems/Damage.hpp"
#include <cstdio>

InventoryUI::InventoryUI(Application* app) : app_(app) {}

void InventoryUI::Toggle() { open_ = !open_; selected_ = 0; }

void InventoryUI::Update(float dt) {
  (void)dt;
  if (!open_) return;
  if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_I) || IsKeyPressed(KEY_ESCAPE)) {
    // ESC handled carefully - only close inv if open
  }
  if (IsKeyPressed(KEY_I)) Toggle();
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) tab_ = (tab_ + 4) % 5;
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) tab_ = (tab_ + 1) % 5;
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) selected_ = std::max(0, selected_ - 1);
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) selected_++;
  if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E)) UseSelected();
}

void InventoryUI::UseSelected() {
  auto& p = *app_->Player();
  auto* ui = app_->UI();
  if (tab_ == 0) {
    if (selected_ < 0 || selected_ >= (int)p.Inv().items.size()) return;
    std::string it = p.Inv().items[selected_];
    if (it.find("Аптечка") != std::string::npos || it.find("medkit") != std::string::npos
        || it.find("энерг") != std::string::npos || it.find("Энерг") != std::string::npos) {
      std::string msg;
      DamageSystem::HealPlayer(p, 45.f, msg);
      p.SetEnergy(std::min(100, p.Energy() + 25));
      ui->Toast(msg);
    } else if (it.find("Отмычка") != std::string::npos) {
      ui->Toast("Отмычка: бонус к rob (пассивно)");
    } else {
      ui->Toast(it);
    }
  } else if (tab_ == 2) {
    if (selected_ < 0 || selected_ >= (int)p.Inv().cars.size()) return;
    p.SetActiveCar(p.Inv().cars[selected_]);
    ui->Toast("Активна: " + p.ActiveCar());
  } else if (tab_ == 1) {
    if (selected_ < 0 || selected_ >= (int)p.Inv().weapons.size()) return;
    ui->Toast("Экипировано: " + p.Inv().weapons[selected_]);
  }
}

void InventoryUI::DrawTab(Rectangle r, int id, const char* name) {
  bool on = tab_ == id;
  DrawRectangleRounded(r, 0.2f, 6, on ? Config::ColAccent() : Config::ColBtn());
  int tw = MeasureText(name, 16);
  DrawText(name, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + 8), 16, Config::ColText());
  if (CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    tab_ = id; selected_ = 0;
  }
}

void InventoryUI::Draw(int sw, int sh) {
  if (!open_) return;
  DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.55f));
  float w = 520, h = 420;
  float x = (sw - w) * 0.5f, y = (sh - h) * 0.5f;
  DrawRectangleRounded({x, y, w, h}, 0.08f, 10, Config::ColPanel2());
  DrawRectangleRoundedLines({x, y, w, h}, 0.08f, 10, Config::ColAccent());
  DrawText("ИНВЕНТАРЬ", (int)x + 20, (int)y + 14, 24, Config::ColText());
  DrawText("A/D вкладки  W/S выбор  E использовать  I закрыть", (int)x + 20, (int)y + 42, 14, Config::ColMuted());

  const char* tabs[] = {"Вещи", "Оружие", "Авто", "Мат-лы", "Одежда"};
  for (int i = 0; i < 5; ++i)
    DrawTab({x + 16 + i * 98.f, y + 68, 90, 32}, i, tabs[i]);

  auto& inv = app_->Player()->Inv();
  std::vector<std::string> lines;
  char buf[128];
  if (tab_ == 0) {
    for (auto& it : inv.items) lines.push_back(it);
  } else if (tab_ == 1) {
    for (auto& it : inv.weapons) lines.push_back(it);
  } else if (tab_ == 2) {
    for (auto& it : inv.cars) {
      std::string s = it;
      if (it == app_->Player()->ActiveCar()) s += " [активна]";
      lines.push_back(s);
    }
  } else if (tab_ == 3) {
    for (auto& [k, v] : inv.materials) {
      if (k.rfind("biz_", 0) == 0 || k.rfind("dmg_", 0) == 0 || k.rfind("cdmg_", 0) == 0
          || k.rfind("sh_", 0) == 0 || k == "drift_mode" || k.rfind("repair_", 0) == 0
          || k.rfind("house_", 0) == 0 || k == "tire_compound") continue;
      snprintf(buf, sizeof(buf), "%s × %d", k.c_str(), v);
      lines.push_back(buf);
    }
  } else {
    for (auto& it : inv.clothes) lines.push_back(it);
  }

  if (selected_ >= (int)lines.size()) selected_ = std::max(0, (int)lines.size() - 1);
  float listY = y + 110;
  DrawRectangleRounded({x + 16, listY, w - 32, h - 130}, 0.06f, 8, Config::ColPanel());
  if (lines.empty()) {
    DrawText("Пусто", (int)x + 30, (int)listY + 20, 18, Config::ColMuted());
  } else {
    int maxShow = 12;
    int start = std::max(0, selected_ - maxShow + 1);
    for (int i = start; i < (int)lines.size() && i < start + maxShow; ++i) {
      float ly = listY + 10 + (i - start) * 26;
      if (i == selected_)
        DrawRectangleRounded({x + 22, ly - 2, w - 44, 24}, 0.2f, 4, Fade(Config::ColAccent(), 0.35f));
      DrawText(lines[i].c_str(), (int)x + 30, (int)ly, 18, Config::ColText());
      Rectangle row = {x + 22, ly - 2, w - 44, 24};
      if (CheckCollisionPointRec(GetMousePosition(), row) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        selected_ = i;
        UseSelected();
      }
    }
  }
}
