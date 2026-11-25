#pragma once
#include "raylib.h"
#include <cmath>
#include <vector>

struct CullSphere {
  Vector3 pos;
  float radius;
  int lod; // 0 near, 1 mid, 2 far
};

class RenderOptimizer {
public:
  static RenderOptimizer& Get() {
    static RenderOptimizer inst;
    return inst;
  }

  void BeginFrame(Camera3D cam) {
    cam_ = cam;
    drawn_ = 0;
    culled_ = 0;
  }

  bool Visible(Vector3 pos, float radius) {
    // simple frustum proxy using camera forward
    Vector3 to = Vector3Subtract(pos, cam_.position);
    float dist = Vector3Length(to);
    if (dist - radius > farPlane_) { culled_++; return false; }
    Vector3 fwd = Vector3Subtract(cam_.target, cam_.position);
    float fl = Vector3Length(fwd);
    if (fl < 0.001f) return true;
    fwd = Vector3Scale(fwd, 1.f / fl);
    float ndot = Vector3DotProduct(Vector3Normalize(to), fwd);
    if (ndot < 0.15f && dist > radius * 2.f) { culled_++; return false; }
    drawn_++;
    return true;
  }

  int LodLevel(Vector3 pos) const {
    float d = Vector3Distance(pos, cam_.position);
    if (d < 40.f) return 0;
    if (d < 100.f) return 1;
    return 2;
  }

  int Drawn() const { return drawn_; }
  int Culled() const { return culled_; }

  void SetFarPlane(float f) { farPlane_ = f; }

private:
  Camera3D cam_{};
  float farPlane_ = 220.f;
  int drawn_ = 0, culled_ = 0;
};
