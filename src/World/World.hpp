#pragma once
#include "City.hpp"
#include "raylib.h"
#include <string>
#include <vector>

struct Building {
  float baseX;
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
  void DrawTraffic(int sw, int sh);
  void DrawPedestrians(int sw, int sh);
  void DrawStreetProps(int sw, int sh);

  std::string cityId_ = "липецк";
  float timeOfDay_ = 14.f;
  float scroll_ = 0.f;
  float trafficT_ = 0.f;
  float pedT_ = 0.f;
  std::vector<Building> buildings_;
  int lastSw_ = 0;
  Color skyTop_{};
  Color skyBot_{};
  float lastSkyT_ = -1.f;
};
