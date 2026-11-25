#pragma once
#include "Systems/DialogueSystem.hpp"
#include "Cutscenes/CutsceneSystem.hpp"
#include "Minigames/StoryMinigames.hpp"
#include "Systems/ReputationScales.hpp"
#include <string>

class StoryDirector {
public:
  static StoryDirector& Get() {
    static StoryDirector inst;
    return inst;
  }

  int Act() const { return act_; }
  void SetAct(int a) { act_ = a; }

  void StartAct1Intro(CutsceneSystem* cuts) {
    act_ = 1;
    if (cuts) cuts->Play("ch1_cold_start");
    DialogueNode n;
    n.speaker = "Seryy";
    n.line = "Bez svyazey zdes' ne zhivut. Beresh' yashchik - ili idesh' peshkom.";
    n.options = {
      {"Zhestko: Ya ne gruzchik", DialogueTone::Hard, 5, -2, 0, "seryy", "disrespect", 1},
      {"Spokoyno: Govori po delu", DialogueTone::Calm, 2, 0, 1, "seryy", "respect", 1},
      {"Naglo: Skol'ko v dolyah?", DialogueTone::Bold, 4, -1, 0, "seryy", "greed", 1},
      {"Molchat'", DialogueTone::Silent, 0, 0, -1, "seryy", "silent", 1},
    };
    pendingDlg_ = n;
    hasPending_ = true;
    pendingMini_ = StoryMiniId::Delivery;
  }

  void AdvanceAct(CutsceneSystem* cuts, StoryMinigames& mini) {
    act_++;
    if (act_ == 2) {
      if (cuts) cuts->Play("ch2_first_wheels");
      pendingMini_ = StoryMiniId::DriftChase;
    } else if (act_ == 3) {
      if (cuts) cuts->Play("ch3_own_corner");
    } else if (act_ == 4) {
      if (cuts) cuts->Play("ch4_dark_deals");
      pendingMini_ = StoryMiniId::CameraRoute;
    } else if (act_ == 5) {
      if (cuts) cuts->Play("ch5_big_stakes");
    } else if (act_ >= 6 && act_ <= 10) {
      char id[32];
      snprintf(id, sizeof(id), "ch%d", act_);
      // map to chapter ids
      const char* ids[] = {"", "ch1_cold_start", "ch2_first_wheels", "ch3_own_corner",
        "ch4_dark_deals", "ch5_big_stakes", "ch6_business", "ch7_setup",
        "ch8_capital", "ch9_war", "ch10_empire"};
      if (act_ >= 1 && act_ <= 10 && cuts) cuts->Play(ids[act_]);
    }
    (void)mini;
  }

  void Poll(DialogueSystem& dlg, StoryMinigames& mini) {
    if (hasPending_ && !dlg.Active()) {
      dlg.Open(pendingDlg_);
      hasPending_ = false;
    }
    if (pendingMini_ != StoryMiniId::None && !dlg.Active() && !mini.Active()) {
      if (pendingMini_ == StoryMiniId::Delivery) mini.StartDelivery();
      if (pendingMini_ == StoryMiniId::DriftChase) mini.StartDriftChase();
      if (pendingMini_ == StoryMiniId::CameraRoute) mini.StartCameraRoute();
      pendingMini_ = StoryMiniId::None;
    }
  }

private:
  int act_ = 0;
  bool hasPending_ = false;
  DialogueNode pendingDlg_{};
  StoryMiniId pendingMini_ = StoryMiniId::None;
};
