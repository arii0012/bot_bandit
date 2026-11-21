#include "UISystem.hpp"
#include "Core/Application.hpp"
#include "Core/Config.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "Systems/GameSystems.hpp"
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
  int tw = MeasureText(label, 18);
  DrawText(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + r.height * 0.5f - 9), 18, Config::ColText());
}

void UISystem::Draw() {
  int sw = app_->ScreenW(), sh = app_->ScreenH();

  // toast
  if (toastTimer_ > 0) {
    int tw = MeasureText(toastMsg_.c_str(), 20) + 40;
    Rectangle r = {(float)(sw - tw) * 0.5f, 80, (float)tw, 40};
    DrawRectangleRounded(r, 0.3f, 8, Config::ColPanel());
    DrawRectangleRoundedLines(r, 0.3f, 8, Config::ColGold());
    DrawText(toastMsg_.c_str(), (int)(r.x + 20), (int)(r.y + 10), 20, Config::ColText());
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
    DrawText(modalTitle_.c_str(), (int)(x + 20), (int)(y + 18), 22, Config::ColText());
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

  // podium
  float cx = sw * 0.5f, cy = sh * 0.55f;
  DrawEllipse((int)cx, (int)(cy + 60), 80, 24, Color{60, 40, 50, 200});
  app_->Player()->Draw(cx, cy);

  DrawText("СОЗДАНИЕ ПЕРСОНАЖА", sw / 2 - 140, 40, 28, Config::ColText());
  DrawText("ЛКМ по кнопкам · Enter — начать", sw / 2 - 130, 78, 18, Config::ColMuted());

  auto& p = *app_->Player();
  float px = 40, py = 140;
  bool h = false;

  auto row = [&](const char* label, float y) {
    DrawText(label, (int)px, (int)y, 18, Config::ColMuted());
  };

  row("Имя: (редактируется в консоли /name)", py);
  char buf[64];
  snprintf(buf, sizeof(buf), "Имя: %s", p.Name().c_str());
  DrawText(buf, (int)px, (int)py + 24, 22, Config::ColText());

  row("Пол", py + 70);
  Rectangle rM = {px, py + 95, 100, 40};
  Rectangle rF = {px + 120, py + 95, 100, 40};
  DrawRoundedButton(rM, "Муж", h, !p.GenderFemale());
  if (h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) p.SetFemale(false);
  DrawRoundedButton(rF, "Жен", h, p.GenderFemale());
  if (h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) p.SetFemale(true);

  row("Причёска", py + 160);
  Rectangle rH = {px, py + 185, 140, 40};
  DrawRoundedButton(rH, "Сменить", h);
  if (h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    p.SetHairStyle((p.HairStyle() + 1) % 6);

  // random colors
  row("Цвет волос", py + 250);
  Color cols[] = { {30,25,20,255}, {180,140,60,255}, {20,20,25,255}, {160,40,40,255}, {200,200,210,255} };
  for (int i = 0; i < 5; ++i) {
    Rectangle rc = {px + i * 44.f, py + 275, 36, 36};
    DrawRectangleRounded(rc, 0.3f, 6, cols[i]);
    if (CheckCollisionPointRec(GetMousePosition(), rc) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
      p.SetHairColor(cols[i]);
  }

  // start
  Rectangle start = {(float)sw * 0.5f - 100, (float)sh - 100, 200, 50};
  DrawRoundedButton(start, "НАЧАТЬ ИГРУ", h, true);
  if ((h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_ENTER)) {
    app_->SetState(AppState::Playing);
    if (app_->Cutscenes()) app_->Cutscenes()->Play("debut");
    Toast("Добро пожаловать в Underworld");
  }
}
