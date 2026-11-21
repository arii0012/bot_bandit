#pragma once
#include "City.hpp"
#include <string>

class World {
public:
  void Update(float dt);
  void Draw(int sw, int sh);

  std::string CurrentCityId() const { return cityId_; }
  std::string CurrentCityName() const;
  const City* Current() const;
  void SetCity(const std::string& id);

  float TimeOfDay() const { return timeOfDay_; } // 0..24

private:
  std::string cityId_ = "липецк";
  float timeOfDay_ = 12.f;
  float anim_ = 0.f;
};
