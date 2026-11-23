#include "CutsceneSystem.hpp"
#include "Core/FontUI.hpp"
#include "raylib.h"
#include <cmath>

void CutsceneSystem::LoadScript(const std::string& id) {
  steps_.clear();
  duration_ = 4.f;
  if (id == "debut") {
    duration_ = 5.f;
    steps_ = {
      {0.0f, "Новый бандит выходит на сцену...", 1.0f},
      {1.5f, "Улицы уже знают твоё имя", 1.05f},
      {3.2f, "Добро пожаловать в UNDERWORLD", 1.1f},
    };
  } else if (id == "keys") {
    duration_ = 4.5f;
    steps_ = {
      {0.0f, "Ключи от мечты в твоих руках", 1.0f},
      {1.8f, "Двигатель рычит. Город ждёт.", 1.12f},
      {3.2f, "Не подведи тачку.", 1.05f},
    };
  } else if (id == "vip") {
    duration_ = 4.f;
    steps_ = {
      {0.0f, "Фейсконтроль...", 1.0f},
      {1.2f, "Дресс-код ок. Проходи.", 1.08f},
      {2.8f, "VIP-зона открыта", 1.15f},
    };
  } else if (id == "travel") {
    duration_ = 3.5f;
    steps_ = {
      {0.0f, "Междугородний рейс", 1.0f},
      {1.5f, "Километры асфальта...", 1.06f},
    };
  } else if (id == "arrest") {
    duration_ = 5.f;
    steps_ = {
      {0.0f, "МИГАЛКИ. ОБЛАВА.", 1.2f},
      {1.5f, "Slow-mo. Руки вверх?", 0.95f},
      {3.2f, "Тепло зашкаливает...", 1.0f},
    };
  } else if (id == "mansion") {
    duration_ = 4.5f;
    steps_ = {
      {0.0f, "Новоселье", 1.0f},
      {1.5f, "Особняк твой. Салют.", 1.1f},
      {3.0f, "Это только начало империи", 1.05f},
    };
  } else if (id == "heist_win") {
    duration_ = 4.f;
    steps_ = {
      {0.0f, "Чисто. Мешки полные.", 1.05f},
      {1.8f, "Уходи, пока не приехали", 1.0f},
    };
  } else {
    steps_ = {{0.f, id, 1.f}};
    duration_ = 3.f;
  }
}

void CutsceneSystem::Play(const std::string& id) {
  id_ = id;
  t_ = 0.f;
  active_ = true;
  zoom_ = 1.f;
  currentText_.clear();
  LoadScript(id);
}

void CutsceneSystem::Update(float dt) {
  if (!active_) return;
  t_ += dt;
  currentText_.clear();
  zoom_ = 1.f;
  for (auto& s : steps_) {
    if (t_ >= s.at) {
      currentText_ = s.text;
      zoom_ = s.zoom;
    }
  }
  if (t_ >= duration_) active_ = false;
}

void CutsceneSystem::Draw(int sw, int sh) {
  if (!active_) return;
  float pulse = sinf(t_ * 3.f) * 0.5f + 0.5f;
  int bar = (int)(68 + pulse * 6);
  DrawRectangle(0, 0, sw, bar, BLACK);
  DrawRectangle(0, sh - bar, sw, bar, BLACK);

  // vignette
  DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.12f + (zoom_ - 1.f) * 0.4f));

  if (!currentText_.empty()) {
    int tw = MeasureText(currentText_.c_str(), 26);
    DrawTextRu(currentText_.c_str(), (sw - tw) / 2, sh - bar + 20, 26, WHITE);
  }
  // progress
  float p = Progress();
  DrawRectangle(0, sh - 4, (int)(sw * p), 4, Color{230, 57, 70, 200});
}
