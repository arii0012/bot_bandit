#pragma once
#include "Character.hpp"

class CharacterCreator {
public:
  void Open(Character& p);
  void Close() { open_ = false; }
  bool IsOpen() const { return open_; }
  void Update(float dt, Character& p);
  void Draw(Character& p, int sw, int sh);
  bool Confirmed() const { return confirmed_; }
  void ClearConfirm() { confirmed_ = false; }

private:
  bool open_ = false;
  bool confirmed_ = false;
  int tab_ = 0;
  float spin_ = 0.f;
};
