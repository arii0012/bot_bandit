#pragma once
#include "raylib.h"

class ExhaustParticles {
public:
  void Reset();
  void Emit(float x, float y, float angle, float intensity, bool nitro);
  void Update(float dt);
  void Draw();

private:
  static constexpr int MAX = 96;
  struct P {
    float x, y, vx, vy, life, maxLife, size;
    unsigned char r, g, b, a;
    bool alive;
  };
  P pool_[MAX]{};
  int cursor_ = 0;
};
