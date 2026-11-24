#include "Core/Application.hpp"
#include "Core/FontUI.hpp"
#include "Core/Config.hpp"
#include "Graphics/AssetManager.hpp"
#include "Graphics/Shaders3D.hpp"
#include "Graphics/ExhaustParticles.hpp"
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
#include "MainMenu.hpp"
#include "CustomCursor.hpp"
#include "Garage3D.hpp"
#include "Mechanics20.hpp"
#include <cstdio>
#include <algorithm>
#include <cmath>

Application::Application() {
  InitWindowBorderless();
  SetTargetFPS(60);
  FontUI::Get().Init();
  HideCursor();

  assets_ = std::make_unique<AssetManager>();
  assets_->Init();
  player_ = std::make_unique<Character>();
  world_ = std::make_unique<World>();
  world_->Init();
  ui_ = std::make_unique<UISystem>(this);
  console_ = std::make_unique<DevConsole>(this);
  saves_ = std::make_unique<SaveSystem>();
  cutscenes_ = std::make_unique<CutsceneSystem>();
  systems_ = std::make_unique<GameSystems>(this);
  phone_ = std::make_unique<Smartphone>();
  weather_ = std::make_unique<WeatherSystem>();
  drift_ = std::make_unique<DriftPhysics>();
  inv_ = std::make_unique<InventoryUI>();
  engineAudio_ = std::make_unique<EngineAudio>();
  exhaust_ = std::make_unique<ExhaustParticles>();
  shaders_ = std::make_unique<Shaders3D>();
  if (shaders_) shaders_->Init();
  mainMenu_ = std::make_unique<MainMenu>();
  cursor_ = std::make_unique<CustomCursor>();
  cursor_->Init();
  garage3d_ = std::make_unique<Garage3D>();
  mech20_ = std::make_unique<Mechanics20>();

  state_ = AppState::MainMenu;
  if (mainMenu_) mainMenu_->Open(saves_->Exists());
}

Application::~Application() {
  if (player_ && world_ && saves_) {
    try { saves_->Save(*player_, *world_); } catch (...) {}
  }
  if (shaders_) shaders_->Shutdown();
  if (engineAudio_) engineAudio_->Stop();
  FontUI::Get().Shutdown();
  if (IsWindowReady()) CloseWindow();
}

void Application::InitWindowBorderless() {
  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_ALWAYS_RUN);
  // Init first so monitor queries work on all platforms / DPI
  InitWindow(1280, 720, "BOT BANDIT: UNDERWORLD");
  int mon = GetCurrentMonitor();
  if (mon < 0) mon = 0;
  screenW_ = GetMonitorWidth(mon);
  screenH_ = GetMonitorHeight(mon);
  if (screenW_ < 640) screenW_ = 1280;
  if (screenH_ < 480) screenH_ = 720;
  SetWindowSize(screenW_, screenH_);
  SetWindowPosition(0, 0);
#if defined(FLAG_BORDERLESS_WINDOWED_MODE)
  // raylib 5.x helper when available
#endif
  SetWindowState(FLAG_WINDOW_UNDECORATED);
  SetWindowPosition(0, 0);
  SetWindowSize(screenW_, screenH_);
  HideCursor();
}

void Application::Run() {
  while (running_) {
    if (WindowShouldClose()) {
      running_ = false;
      break;
    }
    // keep covering monitor (DPI / display change)
    int mon = GetCurrentMonitor();
    int mw = GetMonitorWidth(mon);
    int mh = GetMonitorHeight(mon);
    if (mw > 0 && mh > 0 && (mw != screenW_ || mh != screenH_)) {
      screenW_ = mw;
      screenH_ = mh;
      SetWindowSize(screenW_, screenH_);
      SetWindowPosition(0, 0);
    }

    dt_ = GetFrameTime();
    if (dt_ > 0.1f) dt_ = 0.1f;
    if (dt_ < 0.f) dt_ = 0.f;

    Update();
    BeginDrawing();
    ClearBackground(Color{8, 8, 14, 255});
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
    if (console_) console_->Update();
    return;
  }

  if (state_ == AppState::MainMenu && mainMenu_) {
    auto act = mainMenu_->Update(dt_, *cursor_);
    if (act == MainMenuAction::Continue) {
      if (saves_ && saves_->Exists()) {
        saves_->Load(*player_, *world_);
        state_ = AppState::Playing;
        mainMenu_->Close();
      }
    } else if (act == MainMenuAction::NewGame) {
      player_ = std::make_unique<Character>();
      if (cutscenes_) cutscenes_->Play("debut");
      state_ = AppState::CharacterCreate;
      mainMenu_->Close();
    } else if (act == MainMenuAction::CharacterStudio) {
      state_ = AppState::CharacterCreate;
      mainMenu_->Close();
    } else if (act == MainMenuAction::Garage3D) {
      if (garage3d_) {
        garage3d_->Open(*player_);
        state_ = AppState::Playing;
        mainMenu_->Close();
      }
    } else if (act == MainMenuAction::Settings) {
      if (ui_) ui_->Toast("Settings");
    } else if (act == MainMenuAction::Exit) {
      running_ = false;
    }
    if (IsKeyPressed(KEY_ESCAPE)) running_ = false;
    return;
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    if (inv_ && inv_->IsOpen()) { inv_->Toggle(); return; }
    if (phone_ && phone_->IsOpen()) { phone_->Close(); return; }
    if (garage3d_ && garage3d_->IsOpen()) { garage3d_->Close(); return; }
    if (state_ == AppState::CharacterCreate) {
      state_ = AppState::MainMenu;
      if (mainMenu_) mainMenu_->Open(saves_ && saves_->Exists());
      return;
    }
    if (state_ == AppState::Playing) state_ = AppState::Pause;
    else if (state_ == AppState::Pause) state_ = AppState::Playing;
  }

  if (state_ == AppState::CharacterCreate) {
    if (ui_) ui_->Update(dt_);
    // Enter / Start from creator
    if (IsKeyPressed(KEY_ENTER)) {
      state_ = AppState::Playing;
    }
    return;
  }

  if (state_ == AppState::Pause) return;

  if (state_ == AppState::Playing) {
    if (garage3d_ && garage3d_->IsOpen()) {
      garage3d_->Update(dt_, *player_);
      return;
    }
    if (IsKeyPressed(KEY_G) && garage3d_) garage3d_->Open(*player_);
    if (mech20_) {
      mech20_->NeedsTick(*player_, dt_);
      auto s = mech20_->MiningTick(*player_, dt_);
      if (!s.empty() && ui_) ui_->Toast(s);
    }
    if (systems_) systems_->Update(dt_);
    if (player_) player_->Update(dt_);
    if (world_) world_->Update(dt_);
    if (weather_ && world_) weather_->Update(dt_, world_->TimeOfDay());
    if (cutscenes_) cutscenes_->Update(dt_);

    if (IsKeyPressed(KEY_TAB) && !(inv_ && inv_->IsOpen()) && ui_) ui_->ToggleRadial();
    if (IsKeyPressed(KEY_I) && inv_) inv_->Toggle();
    if (inv_ && inv_->IsOpen()) inv_->Update(dt_);
    else if (ui_) ui_->Update(dt_);

    if (phone_) phone_->Update(dt_);
    if (IsKeyPressed(KEY_P) && phone_) {
      if (phone_->IsOpen()) phone_->Close(); else phone_->Open();
    }
  }
}

void Application::Draw() {
  if (state_ == AppState::MainMenu && mainMenu_) {
    mainMenu_->Draw();
    if (cursor_) { cursor_->Update(0); cursor_->Draw(); }
    return;
  }

  if (state_ == AppState::CharacterCreate) {
    if (ui_) ui_->DrawCharacterCreator();
    if (cursor_) { cursor_->Update(dt_); cursor_->Draw(); }
    FontUI::Get().Draw("ENTER — start game   ESC — menu", 24, (float)screenH_ - 36, 16,
                       Color{160, 160, 180, 255});
    return;
  }

  if (garage3d_ && garage3d_->IsOpen()) {
    garage3d_->Draw(*player_);
    if (cursor_) cursor_->Draw();
    return;
  }

  if (world_) world_->Draw(screenW_, screenH_);
  if (player_) player_->Draw((float)screenW_ * 0.5f, (float)screenH_ * 0.55f);
  if (cutscenes_ && cutscenes_->Active()) cutscenes_->Draw(screenW_, screenH_);

  if (state_ == AppState::Playing || state_ == AppState::Pause || state_ == AppState::Console) {
    DrawHUD();
    if (ui_) ui_->Draw();
    if (inv_ && inv_->IsOpen()) inv_->Draw(screenW_, screenH_);
    if (phone_ && phone_->IsOpen()) phone_->Draw(screenW_, screenH_);
  }

  if (state_ == AppState::Pause) DrawPauseMenu();
  if (state_ == AppState::Console && console_) console_->Draw();

  if (cursor_) { cursor_->Update(0); cursor_->Draw(); }
}

void Application::DrawHUD() {
  if (!player_) return;
  float pad = 16.f;
  char buf[192];
  snprintf(buf, sizeof(buf), "$%lld  LV%d  EN%d  WANTED%d",
           (long long)player_->Money(), player_->Level(), player_->Energy(), player_->Wanted());
  DrawRectangleRounded(Rectangle{pad, pad, 420, 40}, 0.2f, 8, Color{10, 10, 18, 200});
  FontUI::Get().Draw(buf, pad + 12, pad + 10, 18, Color{240, 240, 250, 255});
  FontUI::Get().Draw("TAB radial  I inv  P phone  G garage  ~ console  ESC pause",
                     pad, (float)screenH_ - 28, 14, Color{130, 130, 150, 255});
}

void Application::DrawPauseMenu() {
  int sw = screenW_, sh = screenH_;
  DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 160});
  float w = 360, h = 220;
  float x = (sw - w) * 0.5f, y = (sh - h) * 0.5f;
  DrawRectangleRounded(Rectangle{x, y, w, h}, 0.08f, 10, Color{20, 20, 30, 240});
  FontUI::Get().DrawCentered("PAUSE", x + w * 0.5f, y + 36, 32, WHITE);
  FontUI::Get().DrawCentered("ESC — resume", x + w * 0.5f, y + 100, 18, Color{180, 180, 200, 255});
  FontUI::Get().DrawCentered("Q — quit to menu", x + w * 0.5f, y + 140, 18, Color{180, 180, 200, 255});
  if (IsKeyPressed(KEY_Q)) {
    state_ = AppState::MainMenu;
    if (mainMenu_) mainMenu_->Open(saves_ && saves_->Exists());
  }
}
