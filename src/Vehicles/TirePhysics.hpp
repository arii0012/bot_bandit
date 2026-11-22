#pragma once

// Pacejka-inspired simplified tire: longitudinal + lateral force curves
struct TireParams {
  float muDry = 1.05f;      // friction dry
  float muWet = 0.72f;      // wet
  float load = 1.f;         // normal load factor
  float temp = 60.f;        // °C
  float wear = 0.f;         // 0..1
  float pressure = 2.2f;    // bar
  int compound = 1;         // 0 soft 1 medium 2 hard
};

struct TireForces {
  float longF;  // accel/brake
  float latF;   // cornering
  float slipRatio;
  float slipAngle; // rad
};

class TirePhysics {
public:
  TireParams& P() { return p_; }
  const TireParams& P() const { return p_; }

  void SetSurface(float tractionMult); // from weather
  void UpdateHeat(float dt, float slip, float speed);
  TireForces Compute(float speed, float throttle, float steer, float handbrake);

  // compound shop
  static const char* CompoundName(int c);
  void SetCompound(int c);

private:
  TireParams p_;
  float surfaceMu_ = 1.f;
  float Pacejka(float slip, float B, float C, float D, float E) const;
};
