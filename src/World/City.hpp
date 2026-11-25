#pragma once
#include <string>
#include <vector>

struct Job {
  std::string name;
  int basePay;
  int cooldownSec;
};

struct City {
  std::string id;
  std::string name;
  std::string desc;
  float priceMult = 1.f;
  int workBonus = 0;
  int danger = 1;
  bool hasCasino = false;
  bool hasGym = true;
  bool hasClub = false;
  bool hasAirport = false;
  bool hasDriftTrack = false;
  bool hasAuction = false;
  bool hasStockExchange = false;
  std::vector<Job> jobs;
  float mapX = 0.5f, mapY = 0.5f;
};

const std::vector<City>& AllCities();
const City* FindCity(const std::string& id);

// Playable hub cities with unique mechanics
inline const char* HubCityIds[] = {
  "voronezh", "moskva", "piter", "sochi", "kazan"
};
inline constexpr int HubCityCount = 5;
