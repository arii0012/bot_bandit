#include "Application.hpp"
#include "Core/FontUI.hpp"
#include <algorithm>
#include <cstdio>
#include <cmath>
#include "Config.hpp"
#include "Graphics/AssetManager.hpp"
#include "Graphics/ExhaustParticles.hpp"
#include "Graphics/Shaders3D.hpp"
#include "UI/UISystem.hpp"
#include "UI/InventoryUI.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "DevConsole/DevConsole.hpp"
#include "SaveSystem/SaveSystem.hpp"
#include "Cutscenes/CutsceneSystem.hpp"
#include "Systems/GameSystems.hpp"
#include "Systems/Weather.hpp"
#include "Systems/Damage.hpp"
#include "Systems/EngineAudio.hpp"
#include "Smartphone/Smartphone.hpp"
#include "Vehicles/DriftPhysics.hpp"
#include "Vehicles/CarCustomizer.hpp"

Application::Application() {
  InitWindowBorderless();
  SetTargetFPS(Config::TARGET_FPS);
  SetExitKey(0);
  ShowCursor();
  FontUI::Get().Init();

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
  inv_ = std::make_unique<InventoryUI>(this);
  engineAudio_ = std::make_unique<EngineAudio>();
  exhaust_ = std::make_unique<ExhaustParticles>();
  shaders_ = std::make_unique<Shaders3D>();
  shaders_->Init();

  if (saves_->Exists()) {
    saves_->Load(*player_, *world_);
    state_ = AppState::Playing;
  } else {
    state_ = AppState::CharacterCreate;
  }
}

Application::~Application() {
  if (player_ && world_ && saves_) {
    saves_->Save(*player_, *world_);
  }
  if (shaders_) shaders_->Shutdown();
  if (engineAudio_) engineAudio_->Stop();
  FontUI::Get().Shutdown();
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
  SetWindowSize(screenW_, screenH_);
  if (IsWindowReady()) {
    // ensure covers full monitor
    SetWindowState(FLAG_WINDOW_UNDECORATED);
  }
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
  if (IsKeyPressed(KEY_GRAVE)) {
    if (state_ == AppState::Console) state_ = AppState::Playing;
    else if (state_ == AppState::Playing || state_ == AppState::Pause)
      state_ = AppState::Console;
  }

  if (state_ == AppState::Console) {
    console_->Update();
    return;
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    if (inv_ && inv_->IsOpen()) {
      inv_->Toggle();
      return;
    }
    if (state_ == AppState::Playing) state_ = AppState::Pause;
    else if (state_ == AppState::Pause) state_ = AppState::Playing;
  }

  if (state_ == AppState::CharacterCreate) return;

  if (state_ == AppState::Playing) {
    systems_->Update(dt_);
    player_->Update(dt_);
    world_->Update(dt_);
    if (weather_) weather_->Update(dt_, world_->TimeOfDay());
    if (cutscenes_) cutscenes_->Update(dt_);

    {
      std::string ev;
      DamageSystem::Tick(*player_, dt_, ev);
      if (!ev.empty() && ui_) ui_->Toast(ev);
    }

    if (IsKeyPressed(KEY_TAB) && !(inv_ && inv_->IsOpen())) ui_->ToggleRadial();
    if (IsKeyPressed(KEY_I) && inv_) inv_->Toggle();
    if (inv_ && inv_->IsOpen()) inv_->Update(dt_);
    else ui_->Update(dt_);

    if (phone_) phone_->Update(dt_);
    if (IsKeyPressed(KEY_P) && phone_) {
      if (phone_->IsOpen()) phone_->Close(); else phone_->Open();
    }

    // Drift mode
    if (player_->Inv().materials["drift_mode"] > 0 && drift_) {
      bool a = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
      bool b = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
      bool l = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
      bool r = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
      bool hb = IsKeyDown(KEY_SPACE);
      bool nit = IsKeyDown(KEY_LEFT_SHIFT);
      float trac = weather_ ? weather_->TractionMult() : 1.f;
      drift_->SetTuning(player_->engineLvl_, player_->turboLvl_, trac);
      int comp = 1;
      if (player_->Inv().materials.count("tire_compound"))
        comp = player_->Inv().materials["tire_compound"];
      drift_->Tires().SetCompound(comp);
      drift_->Update(dt_, a, b, l, r, hb, nit);

      if (engineAudio_)
        engineAudio_->Update(fabsf(drift_->State().speed), drift_->State().drifting, nit);
      if (exhaust_) {
        auto& st = drift_->State();
        float inten = std::min(1.f, fabsf(st.speed) / 300.f) + (st.drifting ? 0.4f : 0.f);
        if (fabsf(st.speed) > 20.f || nit)
          exhaust_->Emit(st.x, st.y, st.angle, inten, nit);
        exhaust_->Update(dt_);
      }

      if (IsKeyPressed(KEY_F)) {
        player_->Inv().materials["drift_mode"] = 0;
        if (engineAudio_) engineAudio_->Stop();
        if (exhaust_) exhaust_->Reset();
        ui_->Toast("Дрифт выключен");
      }

      auto& st = drift_->State();
      if (st.x < 40 || st.x > screenW_ - 40 || st.y < 80 || st.y > screenH_ - 40) {
        std::string msg;
        DamageSystem::CrashCar(*player_, 0.35f + fabsf(st.speed) / 800.f, msg);
        st.speed *= -0.3f;
        st.x = std::clamp(st.x, 50.f, (float)screenW_ - 50.f);
        st.y = std::clamp(st.y, 90.f, (float)screenH_ - 50.f);
        if (ui_) ui_->Toast(msg);
      }
      st.speed *= (1.f - (1.f - DamageSystem::PowerMult(*player_)) * 0.02f);
    }
  }
}

void Application::Draw() {
  DrawRectangle(0, 0, screenW_, screenH_, Config::ColBG());

  if (state_ == AppState::CharacterCreate) {
    ui_->DrawCharacterCreator();
    return;
  }

  world_->Draw(screenW_, screenH_);
  if (weather_) weather_->Draw(screenW_, screenH_);

  if (shaders_ && shaders_->Ready()) shaders_->BeginLit();
  player_->Draw(screenW_ * 0.5f, screenH_ * 0.50f);
  if (shaders_ && shaders_->Ready()) shaders_->EndLit();

  if (player_->Inv().materials["drift_mode"] > 0 && drift_) {
    auto& vis = CarCustomizer::Visual(*player_);
    Color body = {vis.r, vis.g, vis.b, 255};
    if (drift_->State().x == 0.f && drift_->State().y == 0.f)
      drift_->Reset(screenW_ * 0.5f, screenH_ * 0.65f, 0.f);
    if (exhaust_) exhaust_->Draw();
    drift_->Draw(0, 0, body, vis.neon || player_->neonCar_);
    char dbuf[96];
    snprintf(dbuf, sizeof(dbuf), "SPD %.0f  SLIP %.0f%%  NITRO %.1f  TIRE %.0fC wear%.0f%%",
             fabsf(drift_->State().speed), drift_->State().slip * 100.f, drift_->State().nitroFuel,
             drift_->Tires().P().temp, drift_->Tires().P().wear * 100.f);
    DrawTextRu(dbuf, 24, screenH_ - 56, 18, Config::ColGold());
  } else {
    player_->DrawCar(screenW_ * 0.5f, screenH_ * 0.72f);
  }

  DrawHUD();
  ui_->Draw();
  if (phone_ && phone_->IsOpen()) phone_->Draw(screenW_, screenH_);
  if (inv_ && inv_->IsOpen()) inv_->Draw(screenW_, screenH_);
  if (cutscenes_ && cutscenes_->Active()) cutscenes_->Draw(screenW_, screenH_);

  if (state_ == AppState::Pause) DrawPauseMenu();
  if (state_ == AppState::Console) console_->Draw();

  Vector2 m = GetMousePosition();
  DrawCircleV(m, 6.f, WHITE);
  DrawCircleV(m, 4.f, Config::ColAccent());
  DrawCircleLines((int)m.x, (int)m.y, 14.f, WHITE);
  DrawCircleLines((int)m.x, (int)m.y, 12.f, Config::ColAccent());
}

void Application::DrawHUD() {
  const float pad = 16.f;
  Rectangle bar = {pad, pad, (float)screenW_ - pad * 2, 48.f};
  DrawRectangleRounded(bar, 0.25f, 8, Config::ColPanel());
  DrawRectangleRoundedLines(bar, 0.25f, 8, Config::ColAccent());

  auto& p = *player_;
  auto& bd = DamageSystem::Body(p);
  char buf[256];
  snprintf(buf, sizeof(buf), "%s  |  $%lld  |  LVL %d  |  EN %d  |  HP %.0f  |  Heat %d  |  %s",
           p.Name().c_str(), (long long)p.Money(), p.Level(), p.Energy(), bd.hp,
           p.Heat(), world_->CurrentCityName().c_str());
  DrawTextRu(buf, (int)(pad + 14), (int)(pad + 14), 20, Config::ColText());

  const char* hints =
      "[1-9] Actions  [I] Inv  [P] Phone  [Tab] Menu  [~] Console  [Esc] Pause";
  DrawTextRu(hints, (int)pad, screenH_ - 28, 16, Config::ColMuted());
}

void Application::DrawPauseMenu() {
  DrawRectangle(0, 0, screenW_, screenH_, Fade(BLACK, 0.65f));
  float w = 360, h = 320;
  float x = (screenW_ - w) * 0.5f, y = (screenH_ - h) * 0.5f;
  Rectangle panel = {x, y, w, h};
  DrawRectangleRounded(panel, 0.12f, 10, Config::ColPanel2());
  DrawRectangleRoundedLines(panel, 0.12f, 10, Config::ColAccent());
  DrawTextRu("PAUSE", (int)(x + 120), (int)(y + 30), 32, Config::ColText());

  auto btn = [&](const char* label, float by, bool& hover) -> bool {
    Rectangle r = {x + 40, y + by, w - 80, 44};
    Vector2 m = GetMousePosition();
    hover = CheckCollisionPointRec(m, r);
    DrawRectangleRounded(r, 0.2f, 8, hover ? Config::ColBtnHover() : Config::ColBtn());
    DrawRectangleRoundedLines(r, 0.2f, 8, Config::ColAccent());
    int tw = MeasureText(label, 20);
    DrawTextRu(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + 12), 20, Config::ColText());
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  };

  bool h1 = false, h2 = false, h3 = false;
  if (btn("Continue", 90, h1)) state_ = AppState::Playing;
  if (btn("Save", 150, h2)) saves_->Save(*player_, *world_);
  if (btn("Exit Game", 210, h3)) {
    saves_->Save(*player_, *world_);
    running_ = false;
  }
}
