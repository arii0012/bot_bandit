#pragma once
#include "Character/Character.hpp"
#include <string>
#include <vector>

class Garage {
public:
  void Open(Character& p);
  void Close();
  bool IsOpen() const;
  void Update(Character& p);
  void Draw(int sw, int sh, Character& p);

private:
  bool open_ = false;
  int sel_ = 0;
  std::vector<std::string> cars_;
};
