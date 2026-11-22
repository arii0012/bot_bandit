#pragma once
#include "raylib.h"
#include <memory>
#include <string>

class AssetManager;
class UISystem;
class Character;
class World;
class DevConsole;
class SaveSystem;
class CutsceneSystem;
class GameSystems;
class Smartphone;
class WeatherSystem;
class DriftPhysics;

enum class AppState {
  Boot,
  CharacterCreate,
  Playing,
  Pause,
  Console
};

class Application {
public:
  Application();
  ~Application();
  void Run();

  int ScreenW() const { return screenW_; }
  int ScreenH() const { return screenH_; }
  AppState State() const { return state_; }
  void SetState(AppState s) { state_ = s; }

  AssetManager* Assets() { return assets_.get(); }
  Character* Player() { return player_.get(); }
  World* GetWorld() { return world_.get(); }
  UISystem* UI() { return ui_.get(); }
  DevConsole* Console() { return console_.get(); }
  SaveSystem* Saves() { return saves_.get(); }
  GameSystems* Systems() { return systems_.get(); }
  Smartphone* Phone() { return phone_.get(); }
  WeatherSystem* Weather() { return weather_.get(); }
  CutsceneSystem* Cutscenes() { return cutscenes_.get(); }

  void RequestExit() { running_ = false; }
  float Dt() const { return dt_; }

private:
  void InitWindowBorderless();
  void Update();
  void Draw();
  void DrawHUD();
  void DrawPauseMenu();

  int screenW_ = 1280;
  int screenH_ = 720;
  bool running_ = true;
  float dt_ = 0.f;
  AppState state_ = AppState::Boot;

  std::unique_ptr<AssetManager> assets_;
  std::unique_ptr<UISystem> ui_;
  std::unique_ptr<Character> player_;
  std::unique_ptr<World> world_;
  std::unique_ptr<DevConsole> console_;
  std::unique_ptr<SaveSystem> saves_;
  std::unique_ptr<CutsceneSystem> cutscenes_;
  std::unique_ptr<GameSystems> systems_;
  std::unique_ptr<Smartphone> phone_;
  std::unique_ptr<WeatherSystem> weather_;
  std::unique_ptr<DriftPhysics> drift_;
};
