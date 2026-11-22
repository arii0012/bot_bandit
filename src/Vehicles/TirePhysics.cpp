#include "TirePhysics.hpp"
#include <cmath>
#include <algorithm>

void TirePhysics::SetSurface(float tractionMult) {
  surfaceMu_ = tractionMult;
}

void TirePhysics::SetCompound(int c) {
  p_.compound = std::clamp(c, 0, 2);
  if (c == 0) { p_.muDry = 1.15f; p_.muWet = 0.55f; }      // soft
  else if (c == 1) { p_.muDry = 1.05f; p_.muWet = 0.72f; } // med
  else { p_.muDry = 0.95f; p_.muWet = 0.80f; }             // hard
}

const char* TirePhysics::CompoundName(int c) {
  if (c == 0) return "Soft";
  if (c == 2) return "Hard";
  return "Medium";
}

void TirePhysics::UpdateHeat(float dt, float slip, float speed) {
  float gen = fabsf(slip) * fabsf(speed) * 0.002f;
  p_.temp += (gen - (p_.temp - 40.f) * 0.15f) * dt;
  p_.temp = std::clamp(p_.temp, 20.f, 140.f);
  // wear
  if (p_.temp > 100.f || fabsf(slip) > 0.5f)
    p_.wear = std::min(1.f, p_.wear + dt * 0.008f * (1.f + fabsf(slip)));
  // overheat loses mu
}

float TirePhysics::Pacejka(float slip, float B, float C, float D, float E) const {
  // simplified magic formula: D*sin(C*atan(B*slip - E*(B*slip - atan(B*slip))))
  float x = B * slip;
  return D * sinf(C * atanf(x - E * (x - atanf(x))));
}

TireForces TirePhysics::Compute(float speed, float throttle, float steer, float handbrake) {
  TireForces f{};
  float mu = (surfaceMu_ < 0.85f ? p_.muWet : p_.muDry) * surfaceMu_;
  mu *= (1.f - p_.wear * 0.35f);
  // temp window peak ~80-95
  float tempFactor = 1.f - fabsf(p_.temp - 88.f) / 120.f;
  tempFactor = std::clamp(tempFactor, 0.55f, 1.1f);
  mu *= tempFactor;
  // pressure
  float presFactor = 1.f - fabsf(p_.pressure - 2.2f) * 0.12f;
  mu *= std::clamp(presFactor, 0.75f, 1.05f);

  float slipRatio = throttle - (speed > 1.f ? 0.15f : 0.f);
  if (handbrake > 0.5f) slipRatio = 0.9f;
  f.slipRatio = slipRatio;
  f.slipAngle = steer * 0.35f * (1.f + handbrake);

  float D = mu * p_.load * 3500.f;
  f.longF = Pacejka(slipRatio, 10.f, 1.6f, D, 0.9f);
  f.latF = Pacejka(f.slipAngle, 8.f, 1.4f, D * 0.95f, 0.95f);
  // friction circle limit
  float comb = sqrtf(f.longF * f.longF + f.latF * f.latF);
  float maxF = D;
  if (comb > maxF && comb > 1.f) {
    float s = maxF / comb;
    f.longF *= s;
    f.latF *= s;
  }
  return f;
}
