#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include "Systems/ReputationScales.hpp"
#include "Systems/NPCMemorySystem.hpp"
#include <string>
#include <vector>
#include <functional>
#include <cmath>

enum class DialogueTone {
  Hard = 0,
  Calm,
  Bold,
  Silent
};

struct DialogueOption {
  const char* label;
  DialogueTone tone;
  int streetDelta;
  int systemDelta;
  int debtDelta;
  const char* memoryNpc; // optional
  const char* memoryFact;
  int memoryDelta;
};

struct DialogueNode {
  const char* speaker;
  const char* line;
  std::vector<DialogueOption> options;
};

class DialogueSystem {
public:
  void Open(const DialogueNode& node) {
    node_ = node;
    active_ = true;
    tension_ = true;
    timer_ = 12.f;
    chosen_ = -1;
  }

  void Close() { active_ = false; tension_ = false; }

  bool Active() const { return active_; }
  bool Tension() const { return tension_ && active_; }

  void Update(float dt) {
    if (!active_) return;
    if (dt > 0.1f) dt = 0.1f;
    // slow-mo feel: still advance timer at reduced pace
    float pace = tension_ ? 0.65f : 1.f;
    timer_ -= dt * pace;
    if (IsKeyPressed(KEY_ONE)) Choose(0);
    if (IsKeyPressed(KEY_TWO)) Choose(1);
    if (IsKeyPressed(KEY_THREE)) Choose(2);
    if (IsKeyPressed(KEY_FOUR)) Choose(3);
    if (timer_ <= 0.f && chosen_ < 0) Choose(3); // silence default if timeout
  }

  void Draw(int sw, int sh) {
    if (!active_) return;
    // darken + vignette tension
    DrawRectangle(0, 0, sw, sh, Fade(BLACK, tension_ ? 0.45f : 0.3f));
    float pulse = 0.5f + 0.5f * sinf(static_cast<float>(GetTime()) * 6.f);

    float boxW = 640.f, boxH = 220.f;
    float x = (sw - boxW) * 0.5f, y = sh - boxH - 90.f;
    DrawRectangleRounded(Rectangle{x, y, boxW, boxH}, 0.08f, 12, Color{14, 14, 24, 245});
    DrawRectangleRoundedLines(Rectangle{x, y, boxW, boxH}, 0.08f, 12,
                              tension_ ? Fade(Color{220, 40, 70, 255}, 0.6f + pulse * 0.4f)
                                       : Color{0, 200, 255, 120});

    FontUI::Get().Draw(node_.speaker, x + 20, y + 14, 18, Color{0, 220, 255, 255});
    FontUI::Get().Draw(node_.line, x + 20, y + 44, 18, WHITE);

    // timer bar
    float tp = timer_ / 12.f;
    if (tp < 0) tp = 0;
    DrawRectangleRec(Rectangle{x + 20, y + 78, 600.f * tp, 4}, Color{255, 60, 90, 220});

    const char* toneLab[] = {"ZHESTKO", "SPOKOYNO", "NAGLO", "PROMOLCHAT"};
    for (int i = 0; i < (int)node_.options.size() && i < 4; ++i) {
      Rectangle r{x + 20 + (i % 2) * 310.f, y + 100.f + (i / 2) * 48.f, 300.f, 40.f};
      bool h = CheckCollisionPointRec(GetMousePosition(), r);
      DrawRectangleRounded(r, 0.2f, 8, h ? Color{255, 55, 95, 255} : Color{32, 32, 48, 230});
      FontUI::Get().DrawCentered(node_.options[i].label ? node_.options[i].label : toneLab[i],
                                 r.x + r.width * 0.5f, r.y + 10.f, 15, WHITE);
      if (h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) Choose(i);
    }
  }

private:
  void Choose(int idx) {
    if (!active_ || idx < 0) return;
    if (idx >= (int)node_.options.size()) {
      // pure silence
      ReputationScales::Get().AddDebt(-2);
      active_ = false;
      tension_ = false;
      chosen_ = 3;
      return;
    }
    const auto& o = node_.options[idx];
    ReputationScales::Get().AddStreet(o.streetDelta);
    ReputationScales::Get().AddSystem(o.systemDelta);
    ReputationScales::Get().AddDebt(o.debtDelta);
    if (o.memoryNpc && o.memoryFact) {
      NPCMemorySystem::Get().Remember(o.memoryNpc, o.memoryFact, o.memoryDelta);
    }
    chosen_ = idx;
    active_ = false;
    tension_ = false;
  }

  bool active_ = false;
  bool tension_ = false;
  float timer_ = 12.f;
  int chosen_ = -1;
  DialogueNode node_{};
};
