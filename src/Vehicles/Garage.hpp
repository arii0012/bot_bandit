#pragma once
#include "Character/Character.hpp"
#include <string>
#include <vector>

struct GarageCarDef {
  std::string id;
  std::string name;
  int price;
};

class Garage {
public:
  void Open(Character& p);
  void Close();
  bool IsOpen() const;
  void Update(Character& p);
  void Draw(int sw, int sh, Character& p);

  static const std::vector<GarageCarDef>& Catalog();
  static bool Buy(Character& p, const std::string& id, std::string& msg);
  static bool Tune(Character& p, const std::string& kind, std::string& msg);

private:
  bool open_ = false;
  int sel_ = 0;
  std::vector<std::string> cars_;
};
