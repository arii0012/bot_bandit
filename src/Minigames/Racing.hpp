#pragma once
#include <string>

class Character;
class WeatherSystem;

class StreetRace {
public:
  // pink-slip style: win opponent car name or money
  static std::string Race(Character& p, WeatherSystem* weather, bool pinkSlip);
};
