#pragma once
#include <string>
#include <functional>

class CutsceneSystem {
public:
  void Play(const std::string& id);
  void Update(float dt);
  void Draw(int sw, int sh);
  bool Active() const { return active_; }

private:
  bool active_ = false;
  float t_ = 0.f;
  std::string id_;
};
