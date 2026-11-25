#pragma once
#include "Character/Character.hpp"
#include "Systems/ReputationScales.hpp"
#include "Core/FontUI.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "raylib.h"
#include <string>
#include <vector>
#include <algorithm>

struct QuestStep {
  const char* desc;
  bool done;
};

struct Quest {
  const char* id;
  const char* title;
  std::vector<QuestStep> steps;
  int64_t rewardRub;
  int streetReward;
  int systemReward;
  int debtReward;
  bool main;
  bool completed;
};

class QuestSystem {
public:
  static QuestSystem& Get() {
    static QuestSystem inst;
    return inst;
  }

  void StartCampaign() {
    quests_.clear();
    quests_.push_back({"q_act1", "Holodnyy asfalt",
      {{"Pogovori s Serym", false}, {"Smena raboty", false}, {"Kopit' na VAZ", false}},
      15000, 5, 0, 2, true, false});
    quests_.push_back({"q_act2", "Pervye kolesa",
      {{"Kupi VAZ/Prioru", false}, {"Nochnoy zaezd", false}},
      25000, 8, 0, 0, true, false});
    quests_.push_back({"q_side_fence", "Sbyt'",
      {{"Prodaj kradenoe baryge", false}},
      8000, 3, -2, 0, false, false});
  }

  void CompleteStep(const std::string& questId, int stepIdx) {
    for (auto& q : quests_) {
      if (q.id != questId) continue;
      if (stepIdx >= 0 && stepIdx < (int)q.steps.size()) q.steps[stepIdx].done = true;
      bool all = true;
      for (auto& s : q.steps) if (!s.done) all = false;
      if (all && !q.completed) {
        q.completed = true;
        pendingReward_ = &q;
      }
      return;
    }
  }

  void ApplyPendingReward(Character& p) {
    if (!pendingReward_) return;
    p.AddMoney(pendingReward_->rewardRub);
    ReputationScales::Get().AddStreet(pendingReward_->streetReward);
    ReputationScales::Get().AddSystem(pendingReward_->systemReward);
    ReputationScales::Get().AddDebt(pendingReward_->debtReward);
    pendingReward_ = nullptr;
  }

  void DrawJournal(int sw, int sh) {
    if (!journalOpen_) return;
    DrawRectangle(0, 0, sw, sh, Color{8, 8, 16, 240});
    FontUI::Get().Draw("ZHURNAL ZADANIY", 40, 30, 26, WHITE);
    float y = 80.f;
    for (auto& q : quests_) {
      Color c = q.completed ? Color{100, 200, 120, 255} : (q.main ? Color{0, 220, 255, 255} : Color{200, 200, 180, 255});
      FontUI::Get().Draw(q.title, 40, y, 18, c);
      y += 24;
      for (auto& s : q.steps) {
        std::string line = std::string(s.done ? "[x] " : "[ ] ") + s.desc;
        FontUI::Get().Draw(line.c_str(), 56, y, 14, s.done ? Color{120, 160, 120, 255} : Color{180, 180, 200, 255});
        y += 20;
      }
      if (!q.completed) {
        FontUI::Get().Draw((std::string("Nagrada ") + ArtworkRenderer::FormatRub(q.rewardRub)).c_str(), 56, y, 13, Color{244, 196, 48, 255});
        y += 22;
      }
      y += 10;
    }
    FontUI::Get().Draw("J - zakryt'", 40, sh - 40.f, 14, Color{140, 140, 160, 255});
  }

  void ToggleJournal() { journalOpen_ = !journalOpen_; }
  bool JournalOpen() const { return journalOpen_; }

  // black swan event
  bool PollBlackSwan(float dt) {
    swanTimer_ -= dt;
    if (swanTimer_ > 0) return false;
    swanTimer_ = 300.f + static_cast<float>(GetRandomValue(0, 200));
    swanMsg_ = (GetRandomValue(0, 2) == 0) ? "REYD NA KLUB" :
               (GetRandomValue(0, 1) ? "BENZIN +40%" : "VOYNA BAND");
    swanShow_ = 6.f;
    return true;
  }

  void DrawSwan(int sw) {
    if (swanShow_ <= 0) return;
    swanShow_ -= GetFrameTime();
    FontUI::Get().DrawCentered(swanMsg_.c_str(), sw * 0.5f, 100, 22, Color{255, 80, 80, 255});
  }

private:
  std::vector<Quest> quests_;
  Quest* pendingReward_ = nullptr;
  bool journalOpen_ = false;
  float swanTimer_ = 120.f;
  float swanShow_ = 0.f;
  std::string swanMsg_;
};
