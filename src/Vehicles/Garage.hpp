#pragma once
#include "Vehicle.hpp"
#include <string>
#include <vector>
#include <map>

struct OwnedCar {
  std::string id;
  std::string name;
  int price = 0;
  float speed = 1.f;
  int tank = 50;
  float fuel = 50.f;
  // tuning
  int engineLvl = 0;
  int turboLvl = 0;
  int suspensionLvl = 0;
  unsigned char colorR = 200, colorG = 40, colorB = 40;
  bool neon = false;
  bool metallic = false;
};

class Character;

class Garage {
public:
  static std::vector<VehicleData> Catalog();
  static bool Buy(Character& p, const std::string& carId, std::string& msg);
  static bool Tune(Character& p, const std::string& what, std::string& msg); // engine|turbo|neon|paint
  static OwnedCar* Active(Character& p);
};
