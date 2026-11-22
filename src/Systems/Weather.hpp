#pragma once
#include "raylib.h"
#include <string>
#include <vector>

enum class WeatherType { Clear, Rain, NeonNight, Fog };

class WeatherSystem {
public:
  void Update(float dt, float timeOfDay);
  void Draw(int sw, int sh);
  WeatherType Type() const { return type_; }
  void Set(WeatherType t);
  void SetNightMode(bool on);
  bool NightMode() const { return nightMode_; }
  float TractionMult() const; // rain -> slippery
  std::string Name() const;

private:
  WeatherType type_ = WeatherType::Clear;
  bool nightMode_ = false;
  float timer_ = 0.f;
  struct Drop { float x, y, spd; };
  std::vector<Drop> rain_;
  void EnsureRain(int sw, int sh);
};
