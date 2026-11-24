#include "Cutscenes/CutsceneSystem.hpp"
#include "Core/FontUI.hpp"
#include <cmath>
#include <cstring>

void CutsceneSystem::EnsureChapters() {
  if (!chapters_.empty()) return;
  chapters_ = {
    {"debut", "Прибытие", 8.f, {
      {0.f, 3.f, "Новый город. Новые правила."},
      {3.f, 6.f, "Здесь всё решает скорость и связи."},
      {6.f, 8.f, "Добро пожаловать в Underworld."},
    }},
    {"first_car", "Первая тачка", 7.f, {
      {0.f, 3.f, "Ключи в руках. Двигатель рычит."},
      {3.f, 7.f, "Город стал ближе на одну передачу."},
    }},
    {"vip_club", "VIP-клуб", 8.f, {
      {0.f, 3.f, "Неон, бас, чужие взгляды."},
      {3.f, 6.f, "Дресс-код пройден. Дверь открыта."},
      {6.f, 8.f, "Здесь заканчиваются случайности."},
    }},
    {"casino_bet", "Крупная ставка", 7.f, {
      {0.f, 3.f, "Шарик катится. Зал замирает."},
      {3.f, 7.f, "Удача любит смелых — или богатых."},
    }},
    {"syndicate", "Синдикат", 9.f, {
      {0.f, 3.f, "Районы делят не на карте."},
      {3.f, 6.f, "Точка за точкой. Кровь за кровь."},
      {6.f, 9.f, "Синдикат запомнил твоё имя."},
    }},
    {"jailbreak", "Побег из КПЗ", 8.f, {
      {0.f, 3.f, "Решётка. Шаги охраны."},
      {3.f, 6.f, "Окно узкое, но достаточно широкое."},
      {6.f, 8.f, "Свобода пахнет бензином."},
    }},
    {"mansion", "Особняк", 7.f, {
      {0.f, 3.f, "Панорамные окна. Тишина внутри."},
      {3.f, 7.f, "Ты купил не стены — статус."},
    }},
    {"finale", "Финал", 10.f, {
      {0.f, 4.f, "Город больше не чужой."},
      {4.f, 7.f, "История не заканчивается — она ускоряется."},
      {7.f, 10.f, "BOT BANDIT: UNDERWORLD"},
    }},
  };
}

void CutsceneSystem::Play(const std::string& id) {
  EnsureChapters();
  chapterIndex_ = -1;
  for (int i = 0; i < (int)chapters_.size(); ++i) {
    if (id == chapters_[i].id) { chapterIndex_ = i; break; }
  }
  if (chapterIndex_ < 0) chapterIndex_ = 0;
  active_ = true;
  t_ = 0.f;
  duration_ = chapters_[chapterIndex_].duration;
  title_ = chapters_[chapterIndex_].title;
  currentText_.clear();
  zoom_ = 1.f;
}

void CutsceneSystem::Update(float dt) {
  if (!active_) return;
  if (dt > 0.1f) dt = 0.1f;
  t_ += dt;
  zoom_ = 1.f + t_ * 0.02f;
  if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
    Skip();
    return;
  }
  if (chapterIndex_ >= 0 && chapterIndex_ < (int)chapters_.size()) {
    currentText_.clear();
    for (auto& ln : chapters_[chapterIndex_].lines) {
      if (t_ >= ln.t0 && t_ < ln.t1) { currentText_ = ln.text; break; }
    }
  }
  if (t_ >= duration_) active_ = false;
}

void CutsceneSystem::Draw(int sw, int sh) {
  if (!active_) return;
  int bar = static_cast<int>(sh * 0.12f);
  DrawRectangle(0, 0, sw, bar, BLACK);
  DrawRectangle(0, sh - bar, sw, bar, BLACK);
  DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.15f + (zoom_ - 1.f) * 0.3f));

  FontUI::Get().DrawCentered(title_.c_str(), sw * 0.5f, static_cast<float>(bar - 36), 22,
                             Color{0, 220, 255, 255});
  if (!currentText_.empty()) {
    FontUI::Get().DrawCentered(currentText_.c_str(), sw * 0.5f, static_cast<float>(sh - bar + 28), 24, WHITE);
  }
  float p = duration_ > 0.f ? t_ / duration_ : 1.f;
  if (p > 1.f) p = 1.f;
  DrawRectangle(0, sh - 4, static_cast<int>(sw * p), 4, Color{230, 57, 70, 200});
  FontUI::Get().Draw("SPACE — пропуск", 24, static_cast<float>(sh - bar + 8), 14, Color{140, 140, 160, 255});
}
