#pragma once
#include <string>
#include <vector>
#include <functional>

struct CutsceneStep {
  float at;          // seconds
  std::string text;
  float zoom;        // 1 = normal
};

class CutsceneSystem {
public:
  void Play(const std::string& id);
  void Update(float dt);
  void Draw(int sw, int sh);
  bool Active() const { return active_; }
  float Progress() const { return duration_ > 0 ? t_ / duration_ : 0.f; }

private:
  bool active_ = false;
  float t_ = 0.f;
  float duration_ = 3.5f;
  std::string id_;
  std::vector<CutsceneStep> steps_;
  std::string currentText_;
  float zoom_ = 1.f;
  void LoadScript(const std::string& id);
};
