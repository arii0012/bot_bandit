#include "Cutscenes/CutsceneSystem.hpp"
#include "Core/FontUI.hpp"
#include <cmath>
#include <cstring>

void CutsceneSystem::EnsureChapters() {
  if (!chapters_.empty()) return;
  chapters_ = {
    {"ch1_cold_start", "Glava 1: Holodnyy start", 10.f, {
      {0.f, 3.f, "Noch. Pereulok. Ty priehal bez svyazey."},
      {3.f, 6.f, "Pere kupshchik u fonarya: 'Rabota est'. Za nal."},
      {6.f, 10.f, "Pervaya podrabotka - stroika ili dostavka."},
    }, {}, "Zarabotay 50 000 P i kupi edy/odezhdu"},

    {"ch2_first_wheels", "Glava 2: Pervye kolesa", 9.f, {
      {0.f, 3.f, "Hvatilo na VAZ. Klyuchi kholodnye."},
      {3.f, 6.f, "Nochnoy zaezd. Signaly. Smeh stritreyserov."},
      {6.f, 9.f, "Teper' u tebya est' kolesa - i dolgi po benzu."},
    }, {}, "Kupi pervoe avto i proedi nochnoy marshrut"},

    {"ch3_own_corner", "Glava 3: Svoy ugol", 9.f, {
      {0.f, 3.f, "Klyuchi ot odnushki. Zapah kraski."},
      {3.f, 6.f, "Nochnaya oblava. Sireny. Nado uhodit'."},
      {6.f, 9.f, "Pervyy opyt pogoni - i pervyy urok."},
    }, {}, "Kupi zhile i ube ri ot politsii"},

    {"ch4_dark_deals", "Glava 4: Temnye dela", 9.f, {
      {0.f, 3.f, "Baryga v podvale: 'Gruz mezhdu gorodami'."},
      {3.f, 6.f, "Risk - konfiskat. Nagrada - zhirnyy nal."},
      {6.f, 9.f, "Vybor: vezti ili slivat' mentam."},
    }, {
      {"Vezti gruz", 4},
      {"Otkazatsya", 4},
    }, "Vypolni kontr bundu ili otkazhis'"},

    {"ch5_big_stakes", "Glava 5: Bol'shie stavki", 10.f, {
      {0.f, 3.f, "VIP-kazino. Avtoritet stavit vse."},
      {3.f, 6.f, "Sharik katitsya. Zal molchit."},
      {6.f, 10.f, "Sdat' ego politsii ili voyti v dolyu?"},
    }, {
      {"Sdat' politsii", 5},
      {"Voyti v dolyu", 5},
    }, "Sygray v kazino protiv avtoriteta"},

    {"ch6_business", "Glava 6: Delovaya hvatka", 9.f, {
      {0.f, 3.f, "Pervaya tocka. Vitrina. Kassovyy apparat."},
      {3.f, 6.f, "Reketyory prosyat 'kryshu'."},
      {6.f, 9.f, "Platit' - ili bit'."},
    }, {
      {"Zaplatit' reketu", 6},
      {"Dat' otpor", 6},
    }, "Kupi biznes i zashchiti tocku"},

    {"ch7_setup", "Glava 7: Podstava", 10.f, {
      {0.f, 3.f, "Naruchniki. Lozhnoe obvinenie."},
      {3.f, 6.f, "Kamera KPZ. Timer na stene."},
      {6.f, 10.f, "Advokat, vzyatka ili pobeg."},
    }, {
      {"Podkup sud'i", 7},
      {"Pobeg", 7},
    }, "Vyyd i iz KPZ lyubym sposobom"},

    {"ch8_capital", "Glava 8: Stolichnye ambitsii", 9.f, {
      {0.f, 3.f, "Moskva / Sochi. Drugoy masshtab."},
      {3.f, 6.f, "Auktsion redkih avto. Elita smotrit."},
      {6.f, 9.f, "Zdes' imya vesit bol'she nalichki."},
    }, {}, "Pereezzhay v stolitsu i zaydi na aukt sion"},

    {"ch9_war", "Glava 9: Voyna sindikatov", 10.f, {
      {0.f, 3.f, "Rayony na karte - krasnye i chernye."},
      {3.f, 6.f, "Tochka za tockoy. Krov' za krov'."},
      {6.f, 10.f, "Reshayushchaya razborka."},
    }, {}, "Zachvati klyuchevye rayony"},

    {"ch10_empire", "Glava 10: Imperiya", 12.f, {
      {0.f, 4.f, "Penthouse. Tishina. Vid na gorod."},
      {4.f, 8.f, "Legal'nyy magnat. Korol' ulits. Kriminal'nyy boss."},
      {8.f, 12.f, "Istoriya ne konchaetsya - ona uskor yaetsya."},
    }, {
      {"Final: Magnat", -1},
      {"Final: Korol' ulits", -1},
      {"Final: Boss", -1},
    }, "Kupi elitnoe zhile i vyberi final"},
  };
}

int CutsceneSystem::ChapterCount() const {
  return 10;
}

const char* CutsceneSystem::CurrentObjective() const {
  if (chapterIndex_ < 0 || chapterIndex_ >= (int)chapters_.size()) return "";
  return chapters_[chapterIndex_].objective ? chapters_[chapterIndex_].objective : "";
}

void CutsceneSystem::Play(const std::string& id) {
  EnsureChapters();
  chapterIndex_ = -1;
  for (int i = 0; i < (int)chapters_.size(); ++i) {
    if (id == chapters_[i].id) { chapterIndex_ = i; break; }
  }
  if (chapterIndex_ < 0) {
    // legacy aliases
    if (id == "debut" || id == "intro") chapterIndex_ = 0;
    else if (id == "first_car") chapterIndex_ = 1;
    else if (id == "vip_club") chapterIndex_ = 4;
    else if (id == "casino_bet") chapterIndex_ = 4;
    else if (id == "syndicate") chapterIndex_ = 8;
    else if (id == "jailbreak") chapterIndex_ = 6;
    else if (id == "mansion" || id == "finale") chapterIndex_ = 9;
    else chapterIndex_ = 0;
  }
  PlayChapter(chapterIndex_);
}

void CutsceneSystem::PlayChapter(int index) {
  EnsureChapters();
  if (index < 0 || index >= (int)chapters_.size()) index = 0;
  chapterIndex_ = index;
  active_ = true;
  pendingChoice_ = false;
  t_ = 0.f;
  duration_ = chapters_[chapterIndex_].duration;
  title_ = chapters_[chapterIndex_].title;
  currentText_.clear();
  zoom_ = 1.f;
}

void CutsceneSystem::SelectChoice(int idx) {
  if (!pendingChoice_ || chapterIndex_ < 0) return;
  auto& ch = chapters_[chapterIndex_];
  if (idx < 0 || idx >= (int)ch.choices.size()) return;
  pendingChoice_ = false;
  active_ = false;
  int next = ch.choices[idx].nextChapter;
  if (next >= 0 && next < (int)chapters_.size()) {
    // stay on same index progression - caller may advance
  }
  (void)next;
}

void CutsceneSystem::Update(float dt) {
  if (!active_) return;
  if (dt > 0.1f) dt = 0.1f;
  t_ += dt;
  zoom_ = 1.f + t_ * 0.02f;
  if (IsKeyPressed(KEY_SPACE) && !pendingChoice_) {
    if (t_ < duration_ - 0.5f) t_ = duration_ - 0.5f;
    else {
      EnsureChapters();
      if (chapterIndex_ >= 0 && !chapters_[chapterIndex_].choices.empty()) {
        pendingChoice_ = true;
      } else {
        Skip();
      }
    }
    return;
  }
  if (IsKeyPressed(KEY_ESCAPE) && !pendingChoice_) { Skip(); return; }

  if (chapterIndex_ >= 0 && chapterIndex_ < (int)chapters_.size()) {
    currentText_.clear();
    for (auto& ln : chapters_[chapterIndex_].lines) {
      if (t_ >= ln.t0 && t_ < ln.t1) { currentText_ = ln.text; break; }
    }
  }
  if (!pendingChoice_ && t_ >= duration_) {
    EnsureChapters();
    if (chapterIndex_ >= 0 && !chapters_[chapterIndex_].choices.empty())
      pendingChoice_ = true;
    else
      active_ = false;
  }
}

void CutsceneSystem::Draw(int sw, int sh) {
  if (!active_ && !pendingChoice_) return;
  int bar = static_cast<int>(sh * 0.12f);
  DrawRectangle(0, 0, sw, bar, BLACK);
  DrawRectangle(0, sh - bar, sw, bar, BLACK);
  DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.15f + (zoom_ - 1.f) * 0.3f));

  FontUI::Get().DrawCentered(title_.c_str(), sw * 0.5f, static_cast<float>(bar - 36), 22,
                             Color{0, 220, 255, 255});
  if (!currentText_.empty() && !pendingChoice_) {
    FontUI::Get().DrawCentered(currentText_.c_str(), sw * 0.5f, static_cast<float>(sh - bar + 28), 22, WHITE);
  }
  float p = duration_ > 0.f ? t_ / duration_ : 1.f;
  if (p > 1.f) p = 1.f;
  DrawRectangle(0, sh - 4, static_cast<int>(sw * p), 4, Color{230, 57, 70, 200});
  FontUI::Get().Draw("SPACE - dalee   ESC - skip", 24, static_cast<float>(sh - bar + 8), 14,
                     Color{140, 140, 160, 255});

  if (pendingChoice_ && chapterIndex_ >= 0) {
    auto& ch = chapters_[chapterIndex_];
    float bx = sw * 0.5f - 160.f;
    float by = sh * 0.5f - ch.choices.size() * 30.f;
    FontUI::Get().DrawCentered("VYBOR", sw * 0.5f, by - 40.f, 24, Color{244, 196, 48, 255});
    for (int i = 0; i < (int)ch.choices.size(); ++i) {
      Rectangle r{bx, by + i * 56.f, 320.f, 48.f};
      bool h = CheckCollisionPointRec(GetMousePosition(), r);
      DrawRectangleRounded(r, 0.2f, 8, h ? Color{255, 60, 90, 255} : Color{32, 32, 48, 230});
      FontUI::Get().DrawCentered(ch.choices[i].label, sw * 0.5f, r.y + 12.f, 18, WHITE);
      if (h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) SelectChoice(i);
      if (IsKeyPressed(KEY_ONE + i)) SelectChoice(i);
    }
  }
}
