#include "Cutscenes/CutsceneSystem.hpp"
#include "Core/FontUI.hpp"
#include <cmath>
#include <cstring>

void CutsceneSystem::EnsureChapters() {
  if (!chapters_.empty()) return;
  chapters_ = {
    {"ch1_cold_start", "Глава 1: Холодный старт", 10.f, {
      {0.f, 3.f, "Ночь. Переулок. Ты приехал без связей."},
      {3.f, 6.f, "Перекупщик у фонаря: «Работа есть. За наличные»."},
      {6.f, 10.f, "Первая подработка — стройка или доставка."},
    }, {}, "Заработай 50 000 ₽ и купи еду или одежду"},

    {"ch2_first_wheels", "Глава 2: Первые колёса", 9.f, {
      {0.f, 3.f, "Хватило на ВАЗ. Ключи холодные."},
      {3.f, 6.f, "Ночной заезд. Сигналы. Смех стритрейсеров."},
      {6.f, 9.f, "Теперь у тебя есть колёса — и долги за бензин."},
    }, {}, "Купи первое авто и проедь ночной маршрут"},

    {"ch3_own_corner", "Глава 3: Свой угол", 9.f, {
      {0.f, 3.f, "Ключи от однушки. Запах краски."},
      {3.f, 6.f, "Ночная облава. Сирены. Нужно уходить."},
      {6.f, 9.f, "Первый опыт погони — и первый урок."},
    }, {}, "Купи жильё и уйди от полиции"},

    {"ch4_dark_deals", "Глава 4: Тёмные дела", 9.f, {
      {0.f, 3.f, "Дельцы в подвале: «Груз между городами»."},
      {3.f, 6.f, "Риск — конфискация. Награда — крупные деньги."},
      {6.f, 9.f, "Выбор: везти или сообщить полиции."},
    }, {
      {"Везти груз", 4},
      {"Отказаться", 4},
    }, "Выполни контрабандное поручение или откажись"},

    {"ch5_big_stakes", "Глава 5: Большие ставки", 10.f, {
      {0.f, 3.f, "VIP-казино. Авторитет ставит всё."},
      {3.f, 6.f, "Шарик катится. Зал молчит."},
      {6.f, 10.f, "Сдать его полиции или войти в долю?"},
    }, {
      {"Сдать полиции", 5},
      {"Войти в долю", 5},
    }, "Сыграй в казино против авторитета"},

    {"ch6_business", "Глава 6: Деловая хватка", 9.f, {
      {0.f, 3.f, "Первая точка. Витрина. Кассовый аппарат."},
      {3.f, 6.f, "Вымогатели требуют «крышу»."},
      {6.f, 9.f, "Платить или дать отпор."},
    }, {
      {"Заплатить вымогателям", 6},
      {"Дать отпор", 6},
    }, "Купи бизнес и защити точку"},

    {"ch7_setup", "Глава 7: Подстава", 10.f, {
      {0.f, 3.f, "Наручники. Ложное обвинение."},
      {3.f, 6.f, "Камера СИЗО. Таймер на стене."},
      {6.f, 10.f, "Адвокат, взятка или побег."},
    }, {
      {"Подкупить судью", 7},
      {"Побег", 7},
    }, "Выйди из СИЗО любым способом"},

    {"ch8_capital", "Глава 8: Столичные амбиции", 9.f, {
      {0.f, 3.f, "Москва или Сочи. Другой масштаб."},
      {3.f, 6.f, "Аукцион редких автомобилей. Элитная публика."},
      {6.f, 9.f, "Здесь имя весит больше наличных."},
    }, {}, "Переезжай в столицу и загляни на аукцион"},

    {"ch9_war", "Глава 9: Война синдикатов", 10.f, {
      {0.f, 3.f, "Районы на карте — красные и чёрные."},
      {3.f, 6.f, "Точка за точкой. Борьба за влияние."},
      {6.f, 10.f, "Решающее противостояние."},
    }, {}, "Получи контроль над ключевыми районами"},

    {"ch10_empire", "Глава 10: Империя", 12.f, {
      {0.f, 4.f, "Пентхаус. Тишина. Вид на город."},
      {4.f, 8.f, "Законный магнат. Король улиц. Криминальный босс."},
      {8.f, 12.f, "История не заканчивается — она ускоряется."},
    }, {
      {"Финал: Магнат", -1},
      {"Финал: Король улиц", -1},
      {"Финал: Босс", -1},
    }, "Купи элитное жильё и выбери финал"},
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
  const int next = ch.choices[idx].nextChapter;
  if (next >= 0 && next < static_cast<int>(chapters_.size())) {
    PlayChapter(next);
  } else {
    active_ = false;
    t_ = 0.f;
  }
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
  FontUI::Get().Draw("ПРОБЕЛ — далее   ESC — пропустить", 24, static_cast<float>(sh - bar + 8), 14,
                     Color{140, 140, 160, 255});

  if (pendingChoice_ && chapterIndex_ >= 0) {
    auto& ch = chapters_[chapterIndex_];
    float bx = sw * 0.5f - 160.f;
    float by = sh * 0.5f - ch.choices.size() * 30.f;
    FontUI::Get().DrawCentered("ВЫБОР", sw * 0.5f, by - 40.f, 24, Color{244, 196, 48, 255});
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
