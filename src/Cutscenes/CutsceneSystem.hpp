#pragma once
#include "raylib.h"
#include <string>
#include <vector>

struct CutLine {
  float t0, t1;
  const char* text;
};

struct StoryChoice {
  const char* label;
  int nextChapter; // -1 = stay / end branch
};

struct CutChapter {
  const char* id;
  const char* title;
  float duration;
  std::vector<CutLine> lines;
  std::vector<StoryChoice> choices; // optional at end
  const char* objective;
};

class CutsceneSystem {
public:
  void Play(const std::string& id);
  void PlayChapter(int index);
  void Update(float dt);
  void Draw(int sw, int sh);
  bool Active() const { return active_; }
  void Skip() { active_ = false; t_ = 0; }
  int CurrentChapter() const { return chapterIndex_; }
  int ChapterCount() const;
  const char* CurrentObjective() const;
  bool HasPendingChoice() const { return pendingChoice_; }
  void SelectChoice(int idx);

private:
  bool active_ = false;
  bool pendingChoice_ = false;
  float t_ = 0.f;
  float duration_ = 1.f;
  std::string currentText_;
  std::string title_;
  float zoom_ = 1.f;
  int chapterIndex_ = -1;
  std::vector<CutChapter> chapters_;
  void EnsureChapters();
};
