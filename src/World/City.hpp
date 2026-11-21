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
  std::vector<Job> jobs;
  float mapX = 0.5f, mapY = 0.5f;
};

const std::vector<City>& AllCities();
const City* FindCity(const std::string& id);
