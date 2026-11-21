#pragma once
#include <string>

class Character;

class FightingClub {
public:
  // returns reward message; modifies player money/energy/exp
  static std::string Fight(Character& p, int stake);
};
