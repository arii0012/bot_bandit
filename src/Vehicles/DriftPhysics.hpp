#pragma once
#include "raylib.h"
#include "TirePhysics.hpp"

struct DriftState {
  float x = 0, y = 0;
  float angle = 0;      // radians heading
  float speed = 0;
  float angularVel = 0;
  float slip = 0;       // 0..1 lateral slip
  bool drifting = false;
  float smokeTimer = 0;
  float tireMarkTimer = 0;
  int engineLvl = 0;
  int turboLvl = 0;
  float traction = 1.f; // weather
  bool nitro = false;
  float nitroFuel = 0;
};

class DriftPhysics {
public:
  void Reset(float x, float y, float angle);
  void SetTuning(int engine, int turbo, float traction);
  TirePhysics& Tires() { return tires_; }
  void Update(float dt, bool accel, bool brake, bool left, bool right, bool handbrake, bool nitroPressed);
  void Draw(float camX, float camY, Color body, bool neon);
  const DriftState& State() const { return s_; }
  DriftState& State() { return s_; }

private:
  DriftState s_;
  TirePhysics tires_;
  static constexpr int MAX_MARKS = 64;
  struct Mark { float x, y, a; float life; };
  Mark marks_[MAX_MARKS]{};
  int markIdx_ = 0;
  void AddMark(float x, float y, float a);
};
