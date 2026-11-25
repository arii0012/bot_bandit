#pragma once
#include "raylib.h"

enum class ParticleKind {
  Exhaust,
  DriftSmoke,
  Spark,
  Rain,
  RainRipple,
  Leaf,
  NeonSpark,
  Muzzle,
  Casing,
  Confetti,
  Dust
};

class ExhaustParticles {
public:
  void Reset();
  void Emit(float x, float y, float angle, float intensity, bool nitro);
  void EmitSparks(float x, float y, int n);
  void EmitDriftSmoke(float x, float y, float angle, float intensity);
  void EmitRain(int sw, int sh, float density);
  void EmitLeaf(float x, float y);
  void EmitMuzzle(float x, float y, float angle);
  void EmitConfetti(float x, float y, int n);
  void EmitNeonSpark(float x, float y);
  void EmitBills(float x, float y, int n);
  void EmitHeatHaze(float x, float y);
  void EmitGlitch(float x, float y, float w, float h);
  void DrawGodRays(float originX, float originY, int sw, int sh, float intensity);
  void Update(float dt);
  void Draw();

private:
  static constexpr int MAX = 512;
  struct P {
    float x, y, vx, vy, life, maxLife, size, angle;
    unsigned char r, g, b, a;
    bool alive;
    ParticleKind kind;
  };
  P pool_[MAX]{};
  int cursor_ = 0;
  P& Next();
};
