#pragma once
#include "raylib.h"
#include <string>
#include <vector>

struct CutLine {
  float t0, t1;
  const char* text;
};

struct CutChapter {
  const char* id;
  const char* title;
  float duration;
  std::vector<CutLine> lines;
};

class CutsceneSystem {
public:
  void Play(const std::string& id);
  void Update(float dt);
  void Draw(int sw, int sh);
  bool Active() const { return active_; }
  void Skip() { active_ = false; t_ = 0; }

private:
  bool active_ = false;
  float t_ = 0.f;
  float duration_ = 1.f;
  std::string currentText_;
  std::string title_;
  float zoom_ = 1.f;
  int chapterIndex_ = -1;
  std::vector<CutChapter> chapters_;
  void EnsureChapters();
};
