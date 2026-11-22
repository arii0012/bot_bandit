#pragma once
#include <string>
#include <vector>
#include "raylib.h"

struct CarVisual {
  unsigned char r = 200, g = 40, b = 40;
  bool neon = false;
  bool metallic = false;
  bool chameleon = false;
  int spoiler = 0;   // 0-2
  int wheels = 0;    // 0-3
  int wrap = 0;      // vinyl 0-4
  int engineLvl = 0;
  int turboLvl = 0;
  int suspension = 0; // 0-2 lower
  bool xenon = false;
  std::string plate;
};

class Character;

class CarCustomizer {
public:
  static CarVisual& Visual(Character& p); // stored in static map by active car name
  static std::string Apply(Character& p, const std::string& opt, std::string& msg);
  static void OpenMenu(class Application* app);
};
