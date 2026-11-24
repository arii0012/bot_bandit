#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <functional>

class Application;

class UISystem {
public:
  explicit UISystem(Application* app);

  void Update(float dt);
  void Draw();
  void DrawCharacterCreator();
  void DrawSettings();
  void DrawShop(int mode); // Shop/Houses/Travel via AppState cast
  void ToggleRadial();
  bool RadialOpen() const { return radialOpen_; }

  void Toast(const std::string& msg);
  void Modal(const std::string& title, const std::vector<std::string>& options,
             std::function<void(int)> onSelect);

private:
  Application* app_;
  bool radialOpen_ = false;
  float radialAnim_ = 0.f;
  std::string toastMsg_;
  float toastTimer_ = 0.f;
  float volMusic_ = 0.8f;
  float volSfx_ = 0.9f;
  float brightness_ = 1.f;
  float camSens_ = 1.f;
  bool shadersOn_ = true;

  bool modalOpen_ = false;
  std::string modalTitle_;
  std::vector<std::string> modalOpts_;
  std::function<void(int)> modalCb_;

  void DrawRoundedButton(Rectangle r, const char* label, bool& outHover, bool accent = false);
  void HandleHotkeys();
};
