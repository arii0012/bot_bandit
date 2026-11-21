#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <functional>

class Application;

enum class PhoneApp {
  Home, Bank, Crypto, DarkNet, Social, Taxi, Music, None
};

class Smartphone {
public:
  explicit Smartphone(Application* app);

  void Open();
  void Close();
  bool IsOpen() const { return open_; }
  void Update(float dt);
  void Draw(int sw, int sh);

private:
  Application* app_;
  bool open_ = false;
  PhoneApp currentApp_ = PhoneApp::Home;
  float anim_ = 0.f;
  std::string statusMsg_;
  float statusTimer_ = 0.f;

  void DrawFrame(Rectangle phone);
  void DrawHome(Rectangle content);
  void DrawBank(Rectangle content);
  void DrawCrypto(Rectangle content);
  void DrawDarkNet(Rectangle content);
  void DrawSocial(Rectangle content);
  void DrawTaxi(Rectangle content);
  bool AppButton(Rectangle r, const char* label, Color accent);
  void SetStatus(const std::string& s);
};
