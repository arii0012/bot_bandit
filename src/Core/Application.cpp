#include "Application.hpp"
#include <algorithm>
#include <cstdio>
#include "Config.hpp"
#include "Graphics/AssetManager.hpp"
#include "UI/UISystem.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "DevConsole/DevConsole.hpp"
#include "SaveSystem/SaveSystem.hpp"
#include "Cutscenes/CutsceneSystem.hpp"
#include "Systems/GameSystems.hpp"
#include "Smartphone/Smartphone.hpp"
#include "Systems/Weather.hpp"
#include "Vehicles/DriftPhysics.hpp"
#include "Vehicles/CarCustomizer.hpp"
#include <cmath>

Application::Application() {
  InitWindowBorderless();
  SetTargetFPS(Config::TARGET_FPS);
  SetExitKey(0);
  HideCursor();

  assets_ = std::make_unique<AssetManager>();
  assets_->LoadAll("assets");

  ui_ = std::make_unique<UISystem>(this);
  player_ = std::make_unique<Character>();
  world_ = std::make_unique<World>();
  console_ = std::make_unique<DevConsole>(this);
  saves_ = std::make_unique<SaveSystem>();
  cutscenes_ = std::make_unique<CutsceneSystem>();
  systems_ = std::make_unique<GameSystems>(this);
  phone_ = std::make_unique<Smartphone>(this);
  weather_ = std::make_unique<WeatherSystem>();
  drift_ = std::make_unique<DriftPhysics>();

  if (saves_->Exists()) {
    saves_->Load(*player_, *world_);
    state_ = AppState::Playing;
  } else {
    state_ = AppState::CharacterCreate;
  }
}

Application::~Application() {
  if (player_ && world_) {
    saves_->Save(*player_, *world_);
  }
  CloseWindow();
}

void Application::InitWindowBorderless() {
  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  int mon = GetCurrentMonitor();
  screenW_ = GetMonitorWidth(mon);
  screenH_ = GetMonitorHeight(mon);
  if (screenW_ < 800) screenW_ = 1280;
  if (screenH_ < 600) screenH_ = 720;
  InitWindow(screenW_, screenH_, "BOT BANDIT: UNDERWORLD");
  SetWindowPosition(0, 0);
}

void Application::Run() {
  while (running_ && !WindowShouldClose()) {
    dt_ = GetFrameTime();
    Update();
    BeginDrawing();
    Draw();
    EndDrawing();
  }
}

void Application::Update() {
  if (IsKeyPressed(KEY_GRAVE)) { // ~
    if (state_ == AppState::Console) state_ = AppState::Playing;
    else if (state_ == AppState::Playing || state_ == AppState::Pause)
      state_ = AppState::Console;
  }

  if (state_ == AppState::Console) {
    console_->Update();
    return;
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    if (state_ == AppState::Playing) state_ = AppState::Pause;
    else if (state_ == AppState::Pause) state_ = AppState::Playing;
    else if (state_ == AppState::CharacterCreate) { /* stay */ }
  }

  if (state_ == AppState::CharacterCreate) {
    // handled in Draw via UI
    return;
  }

  if (state_ == AppState::Playing) {
    systems_->Update(dt_);
    player_->Update(dt_);
    world_->Update(dt_);
    if (weather_) weather_->Update(dt_, world_->TimeOfDay());
    if (cutscenes_) cutscenes_->Update(dt_);
    if (IsKeyPressed(KEY_TAB)) ui_->ToggleRadial();
    ui_->Update(dt_);
    if (phone_) phone_->Update(dt_);
    if (player_->Inv().materials["drift_mode"] > 0 && drift_) {
      bool a = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
      bool b = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
      bool l = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
      bool r = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
      bool hb = IsKeyDown(KEY_SPACE);
      bool nit = IsKeyDown(KEY_LEFT_SHIFT);
      float trac = weather_ ? weather_->TractionMult() : 1.f;
      drift_->SetTuning(player_->engineLvl_, player_->turboLvl_, trac);
      int comp = player_->Inv().materials.count("tire_compound") ? player_->Inv().materials["tire_compound"] : 1;
      drift_->Tires().SetCompound(comp);
      drift_->Update(dt_, a, b, l, r, hb, nit);
      if (IsKeyPressed(KEY_F)) {
        player_->Inv().materials["drift_mode"] = 0;
        ui_->Toast("Дрифт выключен");
      }
    }
    if (IsKeyPressed(KEY_P) && phone_) {
      if (phone_->IsOpen()) phone_->Close(); else phone_->Open();
    }
  }
}

void Application::Draw() {
  // background city tint
  DrawRectangle(0, 0, screenW_, screenH_, Config::ColBG());

  if (state_ == AppState::CharacterCreate) {
    ui_->DrawCharacterCreator();
    return;
  }

  // scene
  world_->Draw(screenW_, screenH_);
  if (weather_) weather_->Draw(screenW_, screenH_);
  player_->Draw(screenW_ * 0.5f, screenH_ * 0.50f);
  if (player_->Inv().materials["drift_mode"] > 0 && drift_) {
    auto& vis = CarCustomizer::Visual(*player_);
    Color body = {vis.r, vis.g, vis.b, 255};
    // center drift on screen
    float dx = drift_->State().x, dy = drift_->State().y;
    if (dx == 0 && dy == 0) drift_->Reset(screenW_ * 0.5f, screenH_ * 0.65f, 0);
    if (exhaust_) exhaust_->Draw();
    drift_->Draw(0, 0, body, vis.neon || player_->neonCar_);
    char dbuf[64];
    snprintf(dbuf, sizeof(dbuf), "SPD %.0f  SLIP %.0f%%  NITRO %.1f  TIRE %.0fC wear%.0f%%",
      fabsf(drift_->State().speed), drift_->State().slip * 100.f, drift_->State().nitroFuel,
      drift_->Tires().P().temp, drift_->Tires().P().wear * 100.f);
    DrawText(dbuf, 24, screenH_ - 56, 18, Config::ColGold());
  } else {
    player_->DrawCar(screenW_ * 0.5f, screenH_ * 0.72f);
  }

  DrawHUD();
  ui_->Draw();
  if (phone_ && phone_->IsOpen()) phone_->Draw(screenW_, screenH_);
  if (cutscenes_ && cutscenes_->Active()) cutscenes_->Draw(screenW_, screenH_);

  if (state_ == AppState::Pause) DrawPauseMenu();
  if (state_ == AppState::Console) console_->Draw();

  // custom cursor
  Vector2 m = GetMousePosition();
  DrawCircleV(m, 4.f, Config::ColAccent());
  DrawCircleLines((int)m.x, (int)m.y, 10.f, Config::ColText());
}

void Application::DrawHUD() {
  const float pad = 16.f;
  // top status bar
  Rectangle bar = {pad, pad, (float)screenW_ - pad * 2, 48.f};
  DrawRectangleRounded(bar, 0.25f, 8, Config::ColPanel());
  DrawRectangleRoundedLines(bar, 0.25f, 8, Config::ColAccent());

  auto& p = *player_;
  char buf[256];
  snprintf(buf, sizeof(buf), "%s  |  $%lld  |  LVL %d  |  HP %d  |  Heat %d  |  %s",
           p.Name().c_str(), (long long)p.Money(), p.Level(), p.Energy(),
           p.Heat(), world_->CurrentCityName().c_str());
  DrawText(buf, (int)(pad + 14), (int)(pad + 14), 20, Config::ColText());

  // bottom action hints
  const char* hints = "[1] Work  [2] Rob  [3] Heist  [4] Travel  [5] Shop  [6] Craft  [8] Garage  [9] Biz  [P] Phone  [~] Console  [Tab] Menu  [~] Console  [Esc] Pause";
  DrawText(hints, (int)pad, screenH_ - 28, 16, Config::ColMuted());
}

void Application::DrawPauseMenu() {
  DrawRectangle(0, 0, screenW_, screenH_, Fade(BLACK, 0.65f));
  float w = 360, h = 320;
  float x = (screenW_ - w) * 0.5f, y = (screenH_ - h) * 0.5f;
  Rectangle panel = {x, y, w, h};
  DrawRectangleRounded(panel, 0.12f, 10, Config::ColPanel2());
  DrawRectangleRoundedLines(panel, 0.12f, 10, Config::ColAccent());

  DrawText("PAUSE", (int)(x + 120), (int)(y + 30), 32, Config::ColText());

  auto btn = [&](const char* label, float by, bool& hover) -> bool {
    Rectangle r = {x + 40, y + by, w - 80, 44};
    Vector2 m = GetMousePosition();
    hover = CheckCollisionPointRec(m, r);
    DrawRectangleRounded(r, 0.2f, 8, hover ? Config::ColBtnHover() : Config::ColBtn());
    DrawRectangleRoundedLines(r, 0.2f, 8, Config::ColAccent());
    int tw = MeasureText(label, 20);
    DrawText(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + 12), 20, Config::ColText());
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  };

  bool h1=false,h2=false,h3=false;
  if (btn("Continue", 90, h1)) state_ = AppState::Playing;
  if (btn("Save", 150, h2)) {
    saves_->Save(*player_, *world_);
  }
  if (btn("Exit Game", 210, h3)) {
    saves_->Save(*player_, *world_);
    running_ = false;
  }
}
