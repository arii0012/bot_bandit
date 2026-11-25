#pragma once
#include "Character/Character.hpp"

class CharacterCreator {
public:
  void Open(Character* p);
  void Close();
  bool IsOpen() const;
  void Update(float dt);
  void Draw(int sw, int sh);

private:
  Character* player_ = nullptr;
  bool open_ = false;
  int tab_ = 0;
  int sel_ = 0;
  float spin_ = 0.f;
};
