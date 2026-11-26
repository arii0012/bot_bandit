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
#include "Graphics/ShaderFX.hpp"
#include "Graphics/TextureAtlasManager.hpp"
#include "Graphics/RenderOptimizer.hpp"
#include "Systems/TutorialSystem.hpp"
#include "Systems/TutorialOverlay.hpp"
#include "UI/ActionPanel.hpp"
#include "Systems/DialogueSystem.hpp"
#include "Systems/StoryDirector.hpp"
#include "Minigames/StoryMinigames.hpp"
#include "Systems/LifeSystems.hpp"
#include "Systems/QuestSystem.hpp"
#include "Systems/PoliceSystem.hpp"
#include "Minigames/ContainerAuction.hpp"
#include "Minigames/CardBluffMinigame.hpp"
#include "Systems/CrimeAndRacketSystem.hpp"
#include "UI/TravelScreen.hpp"
#include "UI/ShopScreen.hpp"
#include "Vehicles/VehicleDeepSystem.hpp"
#include "Systems/ReputationScales.hpp"
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


static ArrestAndJailSystem g_jail;

Application::Application() {
  InitWindowBorderless();
  SetTargetFPS(60);
  FontUI::Get().Init();
  AudioEngine::Get().Init();
  HotReloadConfig::Get().EnsureDefault();
  ModelStore::Get().Init();
  ShaderFX::Get().Init();
  TextureAtlasManager::Get().Init();
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
  TextureAtlasManager::Get().Shutdown();
  ShaderFX::Get().Shutdown();
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
    // jail -> g_jail
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
    if (player_ && state_ == AppState::Playing && !g_jail.Jailed()) {
      float px = (float)screenW_ * 0.5f, py = (float)screenH_ * 0.55f;
      replay.Push(px, py, player_->Facing(), 0.f, player_->Wanted(), GetTime());
      // auto-arrest on high wanted
      if (player_->Wanted() >= 5 && GetRandomValue(0, 1000) < 3) {
        g_jail.Arrest(*player_, CrimeArticle::ResistArrest,
                    static_cast<float>(HotReloadConfig::Get().JailPerStar() * player_->Wanted()));
      }
    }
    if (g_jail.Jailed() && player_) g_jail.Update(dt_, *player_);
    minis.Update(dt_);
    replay.Update(dt_);
    Update();
    BeginDrawing();
    ClearBackground(Color{8, 8, 14, 255});
    Draw();
    if (g_jail.Jailed()) g_jail.Draw(screenW_, screenH_);
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
      StoryDirector::Get().StartAct1Intro(cutscenes_.get());
      QuestSystem::Get().StartCampaign();
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

  if (world_) {
        Camera3D cam = {0};
        cam.position = Vector3{0, 8, 14};
        cam.target = Vector3{0, 1, 0};
        cam.up = Vector3{0, 1, 0};
        cam.fovy = 55.f;
        cam.projection = CAMERA_PERSPECTIVE;
        ClearBackground(Color{8, 12, 28, 255});
        world_->Draw3DScene(cam);
        if (player_) {
          BeginMode3D(cam);
          player_->Draw3D(Vector3{0.f, 0.f, 2.f}, 1.f);
          EndMode3D();
        }
      }
  if (cutscenes_ && cutscenes_->Active()) cutscenes_->Draw(screenW_, screenH_);

  if (state_ == AppState::Playing || state_ == AppState::Pause || state_ == AppState::Console) {
    static TutorialSystem tutorial;
    static TutorialOverlay tutOverlay;
    static ActionPanel actionPanel;
    static DialogueSystem dialogue;
    static StoryMinigames storyMini;
    static FamilyCalls family;
    static RadioSystem radio;
    static FinaleSystem finale;
    static TravelScreen travel;
    static ShopScreen shop;
    static VehicleDeepSystem vehicleDeep;
    static PoliceSystem police;
    static CrimeAndRacketSystem crimeNet;
    static ContainerAuction auction;
    static CardBluffMinigame cards;
    
    static bool tutStarted = false;
    if (!tutStarted && state_ == AppState::Playing) {
      tutorial.Start();
      tutOverlay.Start();
      tutStarted = true;
    }
    tutorial.Update(dt_);
    tutOverlay.Update(dt_);
    StoryDirector::Get().Poll(dialogue, storyMini);
    if (player_) storyMini.Update(dt_, *player_);
    if (player_ && state_ == AppState::Playing) {
      if (shop.IsOpen()) shop.Update(*player_, &vehicleDeep);
      if (travel.IsOpen() && world_) travel.Update(*player_, *world_);
      family.Update(dt_, *player_);
      if (family.Pending()) {
        if (IsKeyPressed(KEY_Y)) family.Accept(*player_);
        if (IsKeyPressed(KEY_N)) family.Refuse();
      }
      if (IsKeyPressed(KEY_R)) { radio.NextWave(); AudioEngine::Get().NextRadio(); }
      if (IsKeyPressed(KEY_J)) QuestSystem::Get().ToggleJournal();
      if (IsKeyPressed(KEY_F7)) auction.Open();
      if (IsKeyPressed(KEY_F8) && player_) cards.Open(25000);
      QuestSystem::Get().PollBlackSwan(dt_);
      crimeNet.TickHotTimer(dt_);
      if (player_) {
        if (auction.IsOpen()) auction.Update(*player_);
        if (cards.Active()) cards.Update(dt_, *player_);
      }
      if (state_ == AppState::Playing && player_) {
        police.Update(dt_, *player_, screenW_, screenH_, crimeNet, g_jail);
      }
      QuestSystem::Get().ApplyPendingReward(*player_);
    }
    dialogue.Update(dt_);
    if (state_ == AppState::Playing && player_ && !tutOverlay.Active()) {
      actionPanel.SetHighlight(ActionId::Count, false);
    } else if (tutOverlay.Active()) {
      actionPanel.SetHighlight(tutOverlay.HighlightAction(), true);
    }
    if (state_ == AppState::Playing && player_) {
      ActionId act = actionPanel.Update(*player_, static_cast<float>(GetTime()));
      if (act == ActionId::Work) {
        storyMini.StartWorkShift("Smena", 8, 5000 + player_->Level() * 500);
      }
      if (act == ActionId::Rob && systems_) systems_->DoRob();
      if (act == ActionId::Fence && systems_) systems_->DoFenceSell();
      if (act == ActionId::LayLow && systems_) systems_->DoHide();
      if (act == ActionId::Rest) {
        player_->SetEnergy(100);
        if (ui_) ui_->Toast("Otdyh: EN polnaya");
      }
      if (IsKeyPressed(KEY_F10) && player_ && StoryDirector::Get().Act() >= 10) {
        // show finale overlay next frames via act check
      }
      if (act == ActionId::Shop) shop.Open();
      if (act == ActionId::Travel) travel.Open();
    }
    DrawHUD();
    if (ui_) ui_->Draw();
    tutorial.Draw(screenW_, screenH_);
    if (state_ == AppState::Playing && player_) {
      actionPanel.Draw(*player_, static_cast<float>(GetTime()), screenW_, screenH_);
      tutOverlay.Draw(screenW_, screenH_);
      dialogue.Draw(screenW_, screenH_);
      storyMini.Draw(screenW_, screenH_);
      family.Draw(screenW_);
      QuestSystem::Get().DrawJournal(screenW_, screenH_);
      QuestSystem::Get().DrawSwan(screenW_);
      if (state_ == AppState::Playing) {
        police.Draw();
        crimeNet.DrawHUD(12.f, 58.f);
        auction.Draw(screenW_, screenH_);
        cards.Draw(screenW_, screenH_);
      }
      if (shop.IsOpen()) shop.Draw(screenW_, screenH_, *player_);
      if (travel.IsOpen() && world_) travel.Draw(screenW_, screenH_, *world_);
      if (garage3d_ && garage3d_->IsOpen()) vehicleDeep.DrawGarageStats(40, 100);
      FontUI::Get().Draw(radio.WaveName(), screenW_ - 200.f, 40, 14, Color{0, 220, 255, 255});
      FontUI::Get().Draw(radio.Blurb(), screenW_ - 280.f, 58, 12, Color{140, 140, 160, 255});
      // scales mini
      if (player_) {
        char sc[80];
        snprintf(sc, sizeof(sc), "УЛИЦА %d  СИСТЕМА %d  ДОЛГ %d",
                 ReputationScales::Get().Street(),
                 ReputationScales::Get().System(),
                 ReputationScales::Get().Debt());
        FontUI::Get().Draw(sc, 12, 40, 14, Color{160, 160, 180, 255});
      }
    }
    if (inv_ && inv_->IsOpen()) inv_->Draw(screenW_, screenH_);
    if (phone_ && phone_->IsOpen()) phone_->Draw(screenW_, screenH_);
  }

  if (state_ == AppState::Pause) DrawPauseMenu();
  if (state_ == AppState::Console && console_) console_->Draw();

  if (cursor_) { cursor_->Update(0); cursor_->Draw(); }
}

void Application::DrawHUD() {
  if (!player_) return;
  float pad = 14.f;
  float x = pad, y = pad;
  auto capsule = [&](const char* text, Color accent) {
    int tw = FontUI::Get().Measure(text, 16) + 28;
    DrawRectangleRounded(Rectangle{x, y, (float)tw, 34.f}, 0.45f, 8, Color{12, 12, 20, 210});
    DrawRectangleRoundedLines(Rectangle{x, y, (float)tw, 34.f}, 0.45f, 8, Fade(accent, 0.55f));
    FontUI::Get().Draw(text, x + 12.f, y + 9.f, 16, WHITE);
    x += tw + 10.f;
  };
  capsule(ArtworkRenderer::FormatRub(player_->Money()).c_str(), Color{244, 196, 48, 255});
  char b[64];
  snprintf(b, sizeof(b), "УР. %d", player_->Level());
  capsule(b, Color{0, 220, 255, 255});
  snprintf(b, sizeof(b), "ЭН %d/100", player_->Energy());
  capsule(b, Color{100, 220, 140, 255});
  if (player_->Wanted() > 0) {
    snprintf(b, sizeof(b), "РОЗЫСК %d", player_->Wanted());
    capsule(b, Color{255, 70, 90, 255});
  }
  // second row scales
  x = pad;
  y = pad + 42.f;
  snprintf(b, sizeof(b), "УЛИЦА %d", ReputationScales::Get().Street());
  capsule(b, Color{220, 80, 100, 255});
  snprintf(b, sizeof(b), "СИСТЕМА %d", ReputationScales::Get().System());
  capsule(b, Color{80, 140, 255, 255});
  snprintf(b, sizeof(b), "ДОЛГ %d", ReputationScales::Get().Debt());
  capsule(b, Color{180, 160, 100, 255});

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
