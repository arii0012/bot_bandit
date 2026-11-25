#pragma once
#include "Systems/DialogueSystem.hpp"
#include "Cutscenes/CutsceneSystem.hpp"
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
  }

  void Poll(DialogueSystem& dlg) {
    if (hasPending_ && !dlg.Active()) {
      dlg.Open(pendingDlg_);
      hasPending_ = false;
    }
  }

private:
  int act_ = 0;
  bool hasPending_ = false;
  DialogueNode pendingDlg_{};
};
