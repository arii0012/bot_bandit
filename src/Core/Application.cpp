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
#include "Systems/AudioEngine.hpp"
#include "Graphics/ModelStore.hpp"
#include "Systems/TutorialSystem.hpp"
#include "Minigames/MinigamesSuite.hpp"
#include "Systems/ArrestAndJailSystem.hpp"
#include "Core/ProfilerOverlay.hpp"
#include "Core/ReplaySystem.hpp"
#include "Core/HotReloadConfig.hpp"
#include "Core/EventBus.hpp"
#include "Systems/DifficultySystem.hpp"
#include "Systems/AntiAbuse.hpp"
#include "Systems/RealisticBusiness.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include <cstdio>
#include <algorithm>
#include <cmath>

Application::Application() {
  InitWindowBorderless();
  SetTargetFPS(60);
  FontUI::Get().Init();
  AudioEngine::Get().Init();
  HotReloadConfig::Get().EnsureDefault();
  ModelStore::Get().Init();
  HideCursor();

  assets_ = std::make_unique<AssetManager>();
  assets_->LoadAll("assets");
  player_ = std::make_unique<Character>();
  world_ = std::make_unique<World>();
  ui_ = std::make_unique<UISystem>(this);
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
  ModelStore::Get().Shutdown();
  AudioEngine::Get().Shutdown();
  FontUI::Get().Shutdown();
  if (IsWindowReady()) CloseWindow();
}

void Application::InitWindowBorderless() {
  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_ALWAYS_RUN);
  // Init first so monitor queries work on all platforms / DPI
  InitWindow(1280, 720, "BOT BANDIT: UNDERWORLD");
  SetExitKey(KEY_NULL);
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
    static ProfilerOverlay prof;
    static ReplaySystem replay;
    static ArrestAndJailSystem jail;
    static MinigamesSuite minis;
    prof.BeginFrame();
    if (IsKeyPressed(KEY_F3)) prof.Toggle();
    if (IsKeyPressed(KEY_F5)) HotReloadConfig::Get().Reload();
    if (IsKeyPressed(KEY_F9)) replay.TogglePlayback();
    AudioEngine::Get().Update(dt_);
    if (state_ == AppState::MainMenu) AudioEngine::Get().SetMood(MusicMood::Menu);
    else if (state_ == AppState::Playing) {
      if (player_ && player_->Wanted() >= 3) AudioEngine::Get().SetMood(MusicMood::Chase);
      else AudioEngine::Get().SetMood(MusicMood::City);
    }
    if (player_ && state_ == AppState::Playing && !jail.Jailed()) {
      float px = (float)screenW_ * 0.5f, py = (float)screenH_ * 0.55f;
      replay.Push(px, py, player_->Facing(), 0.f, player_->Wanted(), GetTime());
      // auto-arrest on high wanted
      if (player_->Wanted() >= 5 && GetRandomValue(0, 1000) < 3) {
        jail.Arrest(*player_, CrimeArticle::ResistArrest,
                    static_cast<float>(HotReloadConfig::Get().JailPerStar() * player_->Wanted()));
      }
    }
    if (jail.Jailed() && player_) jail.Update(dt_, *player_);
    minis.Update(dt_);
    replay.Update(dt_);
    Update();
    BeginDrawing();
    ClearBackground(Color{8, 8, 14, 255});
    Draw();
    if (jail.Jailed()) jail.Draw(screenW_, screenH_);
    if (minis.Active()) minis.Draw(screenW_, screenH_);
    replay.Draw(screenW_, screenH_);
    prof.EndFrame();
    prof.Draw(screenW_);
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
      if (IsKeyDown(KEY_F1)) DifficultySystem::Get().Set(Difficulty::Newbie);
      else if (IsKeyDown(KEY_F2)) DifficultySystem::Get().Set(Difficulty::Street);
      else if (IsKeyDown(KEY_F4)) DifficultySystem::Get().Set(Difficulty::Nineties);
      else if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_F3)) DifficultySystem::Get().Set(Difficulty::Hardcore);
      else DifficultySystem::Get().Set(Difficulty::Street);
      player_ = std::make_unique<Character>();
      DifficultySystem::Get().ApplyNewGame(*player_);
      if (cutscenes_) cutscenes_->Play("ch1_cold_start");
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
      state_ = AppState::Settings;
      mainMenu_->Close();
    } else if (act == MainMenuAction::Exit) {
      running_ = false;
    }
    return;
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    // ESC = back only, never quit
    if (inv_ && inv_->IsOpen()) { inv_->Toggle(); return; }
    if (phone_ && phone_->IsOpen()) { phone_->Close(); return; }
    if (garage3d_ && garage3d_->IsOpen()) { garage3d_->Close(); return; }
    if (state_ == AppState::Settings || state_ == AppState::Shop ||
        state_ == AppState::Houses || state_ == AppState::Travel ||
        state_ == AppState::CharacterCreate) {
      state_ = AppState::MainMenu;
      if (mainMenu_) mainMenu_->Open(saves_ && saves_->Exists());
      return;
    }
    if (state_ == AppState::Pause) { state_ = AppState::Playing; return; }
    if (state_ == AppState::Playing) { state_ = AppState::Pause; return; }
    if (state_ == AppState::MainMenu) { /* stay */ return; }
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

  if (state_ == AppState::Settings) {
    if (ui_) ui_->DrawSettings();
    if (cursor_) { cursor_->Update(dt_); cursor_->Draw(); }
    return;
  }
  if (state_ == AppState::Shop || state_ == AppState::Houses || state_ == AppState::Travel) {
    if (ui_) ui_->DrawShop((int)state_);
    if (cursor_) { cursor_->Update(dt_); cursor_->Draw(); }
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
    static TutorialSystem tutorial;
    static bool tutStarted = false;
    if (!tutStarted && state_ == AppState::Playing) { tutorial.Start(); tutStarted = true; }
    tutorial.Update(dt_);
    DrawHUD();
    if (ui_) ui_->Draw();
    tutorial.Draw(screenW_, screenH_);
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
  /* money via FormatRub */
    snprintf(buf, sizeof(buf), "$%lld  LV%d  EN%d  WANTED%d",
           (long long)player_->Money(), player_->Level(), player_->Energy(), player_->Wanted());
  DrawRectangleRounded(Rectangle{pad, pad, 420, 40}, 0.2f, 8, Color{10, 10, 18, 200});
  FontUI::Get().Draw(buf, pad + 12, pad + 10, 18, Color{240, 240, 250, 255});
  // bottom action bar
  float barH = 52.f;
  DrawRectangle(0, screenH_ - (int)barH, screenW_, (int)barH, Color{10, 10, 18, 220});
  const char* labs[] = {"ТЕЛЕФОН (P)", "ИНВЕНТАРЬ (I)", "ГАРАЖ (G)", "МЕНЮ (ESC)"};
  float bw = 180.f;
  float startX = 16.f;
  for (int i = 0; i < 4; ++i) {
    Rectangle r{startX + i * (bw + 12), screenH_ - barH + 8, bw, 36};
    bool h = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRounded(r, 0.25f, 8, h ? Color{255, 60, 90, 255} : Color{36, 36, 52, 255});
    int tw = FontUI::Get().Measure(labs[i], 14);
    FontUI::Get().Draw(labs[i], r.x + (r.width - tw) * 0.5f, r.y + 10, 14, WHITE);
    if (h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      if (i == 0 && phone_) { if (phone_->IsOpen()) phone_->Close(); else phone_->Open(); }
      if (i == 1 && inv_) inv_->Toggle();
      if (i == 2 && garage3d_) garage3d_->Open(*player_);
      if (i == 3) state_ = AppState::Pause;
    }
  }
}

void Application::DrawPauseMenu() {
  int sw = screenW_, sh = screenH_;
  DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 160});
  float w = 360, h = 220;
  float x = (sw - w) * 0.5f, y = (sh - h) * 0.5f;
  DrawRectangleRounded(Rectangle{x, y, w, h}, 0.08f, 10, Color{20, 20, 30, 240});
  FontUI::Get().DrawCentered("ПАУЗА", x + w * 0.5f, y + 36, 32, WHITE);
  FontUI::Get().DrawCentered("ESC — продолжить", x + w * 0.5f, y + 100, 18, Color{180, 180, 200, 255});
  FontUI::Get().DrawCentered("Q — в главное меню", x + w * 0.5f, y + 140, 18, Color{180, 180, 200, 255});
  if (IsKeyPressed(KEY_Q)) {
    state_ = AppState::MainMenu;
    if (mainMenu_) mainMenu_->Open(saves_ && saves_->Exists());
  }
}
