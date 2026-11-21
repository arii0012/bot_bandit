#pragma once
#include <string>
#include <map>

class Character;
class World;

class BusinessSystem {
public:
  // cityId -> level
  static bool BuyOrUpgrade(Character& p, const World& w, std::string& msg);
  static int Collect(Character& p, const World& w, std::string& msg);
  static int PassiveIncomePerHour(const Character& p);
};
