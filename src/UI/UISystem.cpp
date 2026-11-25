#include "UISystem.hpp"
#include "Core/FontUI.hpp"
#include "UI/LiquidGlassUI.hpp"
#include "Core/Application.hpp"
#include "Core/Config.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "Systems/GameSystems.hpp"
#include "Smartphone/Smartphone.hpp"
#include "Cutscenes/CutsceneSystem.hpp"
#include <cmath>
#include <cstdio>

UISystem::UISystem(Application* app) : app_(app) {}

void UISystem::Update(float dt) {
  if (toastTimer_ > 0) toastTimer_ -= dt;
  if (radialOpen_) radialAnim_ = fminf(1.f, radialAnim_ + dt * 6.f);
  else radialAnim_ = fmaxf(0.f, radialAnim_ - dt * 6.f);

  if (app_->State() == AppState::Playing) HandleHotkeys();
}

void UISystem::HandleHotkeys() {
  auto* sys = app_->Systems();
  if (IsKeyPressed(KEY_ONE)) sys->DoWork(0);
  if (IsKeyPressed(KEY_TWO)) sys->DoRob();
  if (IsKeyPressed(KEY_THREE)) sys->DoHeist();
  if (IsKeyPressed(KEY_FOUR)) sys->OpenTravel();
  if (IsKeyPressed(KEY_FIVE)) sys->OpenShop();
  if (IsKeyPressed(KEY_SIX)) sys->DoScavenge();
  if (IsKeyPressed(KEY_SEVEN)) sys->DoDaily();
  if (IsKeyPressed(KEY_EIGHT)) sys->OpenGarage();
  if (IsKeyPressed(KEY_NINE)) sys->OpenBusiness();
}

void UISystem::ToggleRadial() { radialOpen_ = !radialOpen_; }

void UISystem::Toast(const std::string& msg) {
  toastMsg_ = msg;
  toastTimer_ = 3.0f;
}

void UISystem::Modal(const std::string& title, const std::vector<std::string>& options,
                     std::function<void(int)> onSelect) {
  modalOpen_ = true;
  modalTitle_ = title;
  modalOpts_ = options;
  modalCb_ = std::move(onSelect);
}

void UISystem::DrawRoundedButton(Rectangle r, const char* label, bool& outHover, bool accent) {
  Vector2 m = GetMousePosition();
  outHover = CheckCollisionPointRec(m, r);
  Color bg = accent ? Config::ColAccent() : (outHover ? Config::ColBtnHover() : Config::ColBtn());
  DrawRectangleRounded(r, 0.22f, 8, bg);
  DrawRectangleRoundedLines(r, 0.22f, 8, Config::ColAccent());
  int tw = FontUI::Get().Measure(label, 18);
  DrawTextRu(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + r.height * 0.5f - 9), 18, Config::ColText());
}

void UISystem::Draw() {
  int sw = app_->ScreenW(), sh = app_->ScreenH();
  static float islandPulse = 0.f;
  islandPulse += GetFrameTime() * 2.f;
  if (toastTimer_ > 0.f)
    LiquidGlassUI::DynamicIsland(sw, toastMsg_.c_str(), islandPulse);
  else
    LiquidGlassUI::DynamicIsland(sw, "BOT BANDIT", islandPulse);

  // toast
  if (toastTimer_ > 0) {
    int tw = FontUI::Get().Measure(toastMsg_.c_str(), 20) + 40;
    Rectangle r = {(float)(sw - tw) * 0.5f, 80, (float)tw, 40};
    DrawRectangleRounded(r, 0.3f, 8, Config::ColPanel());
    DrawRectangleRoundedLines(r, 0.3f, 8, Config::ColGold());
    DrawTextRu(toastMsg_.c_str(), (int)(r.x + 20), (int)(r.y + 10), 20, Config::ColText());
  }

  // radial
  if (radialAnim_ > 0.01f) {
    Vector2 c = {(float)sw * 0.5f, (float)sh * 0.5f};
    float R = 120.f * radialAnim_;
    const char* labels[] = {"Work", "Rob", "Heist", "Travel", "Shop", "Craft", "Phone", "Hide"};
    int n = 8;
    for (int i = 0; i < n; ++i) {
      float a = -PI * 0.5f + i * (2 * PI / n);
      float x = c.x + cosf(a) * R;
      float y = c.y + sinf(a) * R;
      Rectangle r = {x - 40, y - 18, 80, 36};
      bool h = false;
      DrawRoundedButton(r, labels[i], h);
      if (h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && radialOpen_) {
        radialOpen_ = false;
        auto* sys = app_->Systems();
        if (i == 0) sys->DoWork(0);
        if (i == 1) sys->DoRob();
        if (i == 2) sys->DoHeist();
        if (i == 3) sys->OpenTravel();
        if (i == 4) sys->OpenShop();
        if (i == 5) sys->OpenCraft();
        if (i == 6) { if (app_->Phone()) app_->Phone()->Open(); }
        if (i == 7) sys->DoHide();
      }
    }
  }

  // modal
  if (modalOpen_) {
    DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.55f));
    float w = 420, h = 80.f + modalOpts_.size() * 52.f;
    float x = (sw - w) * 0.5f, y = (sh - h) * 0.5f;
    DrawRectangleRounded({x, y, w, h}, 0.1f, 10, Config::ColPanel2());
    DrawRectangleRoundedLines({x, y, w, h}, 0.1f, 10, Config::ColAccent());
    DrawTextRu(modalTitle_.c_str(), (int)(x + 20), (int)(y + 18), 22, Config::ColText());
    for (size_t i = 0; i < modalOpts_.size(); ++i) {
      Rectangle r = {x + 30, y + 60 + i * 52, w - 60, 42};
      bool h = false;
      DrawRoundedButton(r, modalOpts_[i].c_str(), h);
      if (h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        modalOpen_ = false;
        if (modalCb_) modalCb_((int)i);
      }
    }
  }
}

void UISystem::DrawCharacterCreator() {
  int sw = app_->ScreenW(), sh = app_->ScreenH();
  DrawRectangle(0, 0, sw, sh, Config::ColBG());

  auto& p = *app_->Player();

  // rotate with drag or buttons
  static bool dragging = false;
  static float lastMX = 0;
  Vector2 m = GetMousePosition();
  float cx = sw * 0.55f, cy = sh * 0.52f;
  Rectangle podium = {cx - 100, cy - 20, 200, 160};
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(m, podium)) {
    dragging = true; lastMX = m.x;
  }
  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragging = false;
  if (dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    p.Facing() += (m.x - lastMX) * 0.02f;
    lastMX = m.x;
  }

  // neon podium
  DrawEllipse((int)cx, (int)(cy + 70), 90, 28, Fade(Config::ColAccent(), 0.35f));
  DrawEllipse((int)cx, (int)(cy + 70), 70, 18, Fade(Color{244,196,48,255}, 0.25f));
  p.Draw(cx, cy);
  p.DrawCar(cx, cy + 110);

  DrawTextRu("СТУДИЯ ПЕРСОНАЖА 360°", 40, 36, 28, Config::ColText());
  DrawTextRu("Тяни мышью по подиуму — поворот | Q/E — шаг", 40, 70, 16, Config::ColMuted());

  if (IsKeyDown(KEY_Q)) p.Facing() -= 0.04f;
  if (IsKeyDown(KEY_E)) p.Facing() += 0.04f;

  float px = 40, py = 110;
  bool h = false;
  auto btn = [&](Rectangle r, const char* lab, bool on) -> bool {
    DrawRoundedButton(r, lab, h, on);
    return h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  };

  DrawTextRu("Пол", (int)px, (int)py, 18, Config::ColMuted());
  if (btn({px, py+22, 90, 36}, "Муж", !p.GenderFemale())) p.SetFemale(false);
  if (btn({px+100, py+22, 90, 36}, "Жен", p.GenderFemale())) p.SetFemale(true);

  DrawTextRu("Причёска", (int)px, (int)py+70, 18, Config::ColMuted());
  if (btn({px, py+92, 120, 36}, "Сменить", false)) p.SetHairStyle((p.HairStyle()+1)%8);

  DrawTextRu("Цвет волос", (int)px, (int)py+140, 18, Config::ColMuted());
  Color cols[] = {{30,25,20,255},{180,140,60,255},{20,20,25,255},{160,40,40,255},{200,200,210,255},{80,40,120,255}};
  for (int i = 0; i < 6; ++i) {
    Rectangle rc = {px + i*42.f, py+162, 34, 34};
    DrawRectangleRounded(rc, 0.3f, 6, cols[i]);
    if (CheckCollisionPointRec(m, rc) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
      p.SetHairColor(cols[i]);
  }

  DrawTextRu("Пресеты", (int)px, (int)py+210, 18, Config::ColMuted());
  if (btn({px, py+232, 90, 32}, "Улица", false)) {
    p.SetFemale(false); p.SetHairStyle(2); p.SetHairColor({20,20,25,255});
  }
  if (btn({px+95, py+232, 90, 32}, "Кибер", false)) {
    p.SetFemale(false); p.SetHairStyle(5); p.SetHairColor({80,40,120,255});
  }
  if (btn({px+190, py+232, 90, 32}, "Бизнес", false)) {
    p.SetFemale(false); p.SetHairStyle(0); p.SetHairColor({30,25,20,255});
  }
  if (btn({px+285, py+232, 90, 32}, "Блондин", false)) {
    p.SetFemale(false); p.SetHairStyle(1); p.SetHairColor({200,180,100,255});
  }
  if (btn({px, py+270, 90, 32}, "Модель", false)) {
    p.SetFemale(true); p.SetHairStyle(3); p.SetHairColor({30,25,20,255});
  }
  if (btn({px+95, py+270, 90, 32}, "Рокерша", false)) {
    p.SetFemale(true); p.SetHairStyle(2); p.SetHairColor({160,40,40,255});
  }
  if (btn({px+190, py+270, 90, 32}, "Блондинка", false)) {
    p.SetFemale(true); p.SetHairStyle(1); p.SetHairColor({220,200,120,255});
  }
  if (btn({px+285, py+270, 90, 32}, "Случайный", false)) {
    p.SetFemale(GetRandomValue(0,1)==1);
    p.SetHairStyle(GetRandomValue(0,7));
    p.SetHairColor(cols[GetRandomValue(0,5)]);
    p.Facing() = GetRandomValue(0,628)/100.f;
  }

  // angle indicator
  char abuf[32];
  snprintf(abuf, sizeof(abuf), "Угол: %.0f°", p.Facing() * 180.f / 3.14159f);
  DrawTextRu(abuf, (int)(cx - 40), (int)(cy + 150), 16, Config::ColMuted());

  Rectangle start = {(float)sw * 0.5f - 110, (float)sh - 90, 220, 52};
  if (btn(start, "НАЧАТЬ ИГРУ", true) || IsKeyPressed(KEY_ENTER)) {
    app_->SetState(AppState::Playing);
    if (app_->Cutscenes()) app_->Cutscenes()->Play("debut");
    Toast("Добро пожаловать в Underworld");
  }
  Rectangle quit = {(float)sw * 0.5f - 110, (float)sh - 40, 220, 36};
  if (btn(quit, "ВЫХОД", false)) {
    app_->RequestExit();
  }
}



#include "ShopCatalog.hpp"
#include "Systems/AudioEngine.hpp"

void UISystem::DrawSettings() {
  int sw = app_->ScreenW(), sh = app_->ScreenH();
  DrawRectangle(0, 0, sw, sh, Color{8, 8, 14, 255});
  float panelW = 520, panelH = 420;
  float x = (sw - panelW) * 0.5f, y = (sh - panelH) * 0.5f;
  DrawRectangleRounded(Rectangle{x, y, panelW, panelH}, 0.06f, 12, Color{22, 22, 34, 240});
  DrawRectangleRoundedLines(Rectangle{x, y, panelW, panelH}, 0.06f, 12, Color{220, 40, 70, 255});
  DrawTextRu("НАСТРОЙКИ", (int)(x + 28), (int)(y + 24), 28, WHITE);
  DrawTextRu("ESC — назад", (int)(x + 28), (int)(y + 58), 16, Color{140, 140, 160, 255});

  auto slider = [&](float sx, float sy, const char* lab, float* val) {
    DrawTextRu(lab, (int)sx, (int)sy, 18, Color{200, 200, 210, 255});
    Rectangle track{sx, sy + 28, 360, 14};
    DrawRectangleRounded(track, 0.5f, 4, Color{40, 40, 55, 255});
    DrawRectangleRounded(Rectangle{track.x, track.y, track.width * (*val), track.height}, 0.5f, 4, Color{220, 40, 70, 255});
    Vector2 m = GetMousePosition();
    if (CheckCollisionPointRec(m, Rectangle{track.x, track.y - 8, track.width, track.height + 16}) &&
        IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      *val = (m.x - track.x) / track.width;
      if (*val < 0) *val = 0;
      if (*val > 1) *val = 1;
    }
  };
  slider(x + 40, y + 100, "Громкость музыки", &volMusic_);
  AudioEngine::Get().SetMusicVolume(volMusic_);
  slider(x + 40, y + 160, "Громкость эффектов", &volSfx_);
  AudioEngine::Get().SetSfxVolume(volSfx_);
  slider(x + 40, y + 220, "Яркость", &brightness_);
  slider(x + 40, y + 280, "Чувствительность камеры", &camSens_);

  Rectangle tog{x + 40, y + 340, 200, 40};
  bool h = CheckCollisionPointRec(GetMousePosition(), tog);
  DrawRectangleRounded(tog, 0.25f, 8, shadersOn_ ? Color{40, 160, 90, 255} : Color{50, 50, 70, 255});
  DrawTextRu(shadersOn_ ? "Шейдеры ВКЛ" : "Шейдеры ВЫКЛ", (int)(tog.x + 24), (int)(tog.y + 10), 18, WHITE);
  if (h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) shadersOn_ = !shadersOn_;

  Rectangle back{x + panelW - 160, y + panelH - 56, 130, 40};
  h = CheckCollisionPointRec(GetMousePosition(), back);
  DrawRectangleRounded(back, 0.25f, 8, h ? Color{255, 60, 90, 255} : Color{220, 40, 70, 255});
  DrawTextRu("НАЗАД", (int)(back.x + 40), (int)(back.y + 10), 18, WHITE);
  if (h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    app_->SetState(AppState::MainMenu);
  }
}

void UISystem::DrawShop(int mode) {
  int sw = app_->ScreenW(), sh = app_->ScreenH();
  DrawRectangle(0, 0, sw, sh, Color{10, 10, 16, 255});
  const char* title = "SHOP";
  const std::vector<ShopItem>* cat = &CatalogCars();
  if (mode == (int)AppState::Houses) { title = "HOUSES"; cat = &CatalogHouses(); }
  else if (mode == (int)AppState::Travel) { title = "TRAVEL"; cat = nullptr; }
  else if (mode == (int)AppState::Shop) { title = "SHOP"; cat = &CatalogClothes(); }

  DrawTextRu(title, 40, 30, 32, WHITE);
  DrawTextRu("ESC — назад", 40, 70, 16, Color{140, 140, 160, 255});
  auto* p = app_->Player();
  char money[64];
  snprintf(money, sizeof(money), "$%lld", p ? (long long)p->Money() : 0);
  DrawTextRu(money, sw - 200, 30, 24, Color{244, 196, 48, 255});

  if (mode == (int)AppState::Travel) {
    for (int i = 0; i < CityCount; ++i) {
      int col = i % 3, row = i / 3;
      Rectangle r{(float)(40 + col * 280), (float)(110 + row * 70), 260, 56};
      bool h = CheckCollisionPointRec(GetMousePosition(), r);
      DrawRectangleRounded(r, 0.2f, 8, h ? Color{60, 50, 80, 255} : Color{30, 30, 45, 255});
      DrawRectangleRoundedLines(r, 0.2f, 8, Color{0, 200, 255, 120});
      DrawTextRu(CityList[i], (int)(r.x + 16), (int)(r.y + 16), 20, WHITE);
      if (h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && p) {
        p->SetCity(CityList[i]);
        Toast(std::string("Travel -> ") + CityList[i]);
        app_->SetState(AppState::Playing);
      }
    }
    return;
  }

  if (!cat) return;
  float scroll = 0.f;
  for (size_t i = 0; i < cat->size(); ++i) {
    const auto& it = (*cat)[i];
    float y = 110.f + i * 72.f - scroll;
    if (y > sh) break;
    Rectangle card{40, y, (float)sw - 80, 64};
    DrawRectangleRounded(card, 0.12f, 8, Color{24, 24, 36, 230});
    DrawRectangleRoundedLines(card, 0.12f, 8, Color{220, 40, 70, 100});
    DrawTextRu(it.name, (int)(card.x + 20), (int)(card.y + 12), 20, WHITE);
    char pr[32];
    snprintf(pr, sizeof(pr), "$%d", it.price);
    DrawTextRu(pr, (int)(card.x + 20), (int)(card.y + 36), 16, Color{244, 196, 48, 255});
    Rectangle buy{card.x + card.width - 140, card.y + 12, 120, 40};
    bool h = CheckCollisionPointRec(GetMousePosition(), buy);
    DrawRectangleRounded(buy, 0.25f, 8, h ? Color{255, 60, 90, 255} : Color{220, 40, 70, 255});
    DrawTextRu("BUY", (int)(buy.x + 36), (int)(buy.y + 10), 18, WHITE);
    if (h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && p) {
      if (p->Money() >= it.price) {
        p->AddMoney(-it.price);
        if (std::string(it.kind) == "clothes") p->Inv().clothes.push_back(it.id);
        else if (std::string(it.kind) == "cars") { p->Inv().cars.push_back(it.name); p->SetActiveCar(it.name); }
        else if (std::string(it.kind) == "weapons") p->Inv().weapons.push_back(it.name);
        else if (std::string(it.kind) == "houses") p->Inv().items.push_back(it.id);
        Toast(std::string("Bought ") + it.name);
      } else Toast("Not enough money");
    }
  }
}

void UISystem::OpenShop() {
  Toast("Magazin otkryt");
  // shop panel uses existing shop UI if any
}
