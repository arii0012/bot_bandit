#pragma once
#include "raylib.h"

// Lightweight 2.5D / faux-3D shaders (no external mesh required)
class Shaders3D {
public:
  void Init();
  void Shutdown();
  void BeginLit();
  void EndLit();
  void BeginOutline();
  void EndOutline();
  bool Ready() const { return ready_; }

private:
  Shader lit_{};
  Shader outline_{};
  bool ready_ = false;
  int locLightDir_ = -1;
  int locAmbient_ = -1;
};
