#pragma once
#include "City.hpp"
#include "raylib.h"
#include <string>
#include <vector>

struct Building {
  float baseX;   // 0..1 of screen width cycle
  int width;
  int height;
  int seed;
};

class World {
public:
  World();
  void Update(float dt);
  void Draw(int sw, int sh);

  std::string CurrentCityId() const { return cityId_; }
  std::string CurrentCityName() const;
  const City* Current() const;
  void SetCity(const std::string& id);

  float TimeOfDay() const { return timeOfDay_; }

private:
  void EnsureBuildings(int sw);
  void DrawSky(int sw, int sh);
  void DrawBuildings(int sw, int sh);
  void DrawGround(int sw, int sh);

  std::string cityId_ = "липецк";
  float timeOfDay_ = 12.f;
  float scroll_ = 0.f;
  std::vector<Building> buildings_;
  int lastSw_ = 0;
  // cached sky colors
  Color skyTop_{}, skyBot_{};
  float lastSkyT_ = -1.f;
};
