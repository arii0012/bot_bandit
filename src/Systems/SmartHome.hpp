#pragma once
#include <string>

class Character;

class SmartHome {
public:
  static bool HasMansion(const Character& p);
  static std::string ToggleLight(Character& p);
  static std::string ToggleAlarm(Character& p);
  static std::string OpenSafe(Character& p);
  static std::string Party(Character& p);
  static std::string OrderGear(Character& p, const std::string& kind);
};
