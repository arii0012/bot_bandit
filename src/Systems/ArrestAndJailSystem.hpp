#pragma once
#include "Character/Character.hpp"
#include "Core/FontUI.hpp"
#include "Core/EventBus.hpp"
#include "raylib.h"
#include <string>
#include <algorithm>

enum class CrimeArticle {
  None,
  Drugs,
  IllegalWeapon,
  CarTheft,
  Shooting,
  ResistArrest
};

class ArrestAndJailSystem {
public:
  void Arrest(Character& p, CrimeArticle art, float seconds) {
    if (jailed_) return;
    article_ = art;
    jailTime_ = seconds;
    jailMax_ = seconds;
    jailed_ = true;
    // confiscate illegal
    if (art == CrimeArticle::IllegalWeapon || art == CrimeArticle::Shooting)
      p.Inv().weapons.clear();
    if (art == CrimeArticle::Drugs) {
      // strip stim tags if any in items
      p.Inv().items.erase(
          std::remove_if(p.Inv().items.begin(), p.Inv().items.end(),
                         [](const std::string& s) {
                           return s.find("stim") != std::string::npos || s.find("drug") != std::string::npos;
                         }),
          p.Inv().items.end());
    }
    p.SetWanted(0);
    EventBus::Get().Emit(GameEvent::PlayerArrested, EventPayload{static_cast<int64_t>(art), (int64_t)seconds});
  }

  void Update(float dt, Character& p) {
    if (!jailed_) return;
    jailTime_ -= dt;
    if (jailTime_ <= 0.f) Release(p);
    if (IsKeyPressed(KEY_B)) TryBribe(p);
    if (IsKeyPressed(KEY_L)) TryLawyer(p);
    if (IsKeyPressed(KEY_E)) workReduce_ = true;
    if (workReduce_) {
      jailTime_ -= dt * 1.5f;
    }
  }

  void Draw(int sw, int sh) {
    if (!jailed_) return;
    DrawRectangle(0, 0, sw, sh, Color{10, 12, 18, 240});
    // cell bars
    for (int i = 0; i < 12; ++i)
      DrawRectangle(80 + i * 50, 80, 10, sh - 160, Color{60, 60, 70, 255});
    FontUI::Get().DrawCentered("KPZ / JAIL", sw * 0.5f, 40, 28, WHITE);
    char buf[128];
    snprintf(buf, sizeof(buf), "Article: %s", ArticleName());
    FontUI::Get().DrawCentered(buf, sw * 0.5f, 90, 18, Color{255, 180, 80, 255});
    snprintf(buf, sizeof(buf), "Time left: %.0f sec", jailTime_);
    FontUI::Get().DrawCentered(buf, sw * 0.5f, 130, 22, Color{0, 220, 255, 255});
    FontUI::Get().DrawCentered("B - bribe   L - lawyer   E - work reduce", sw * 0.5f, sh - 80, 16,
                               Color{160, 160, 180, 255});
    float p = jailMax_ > 0 ? (1.f - jailTime_ / jailMax_) : 1.f;
    DrawRectangleRec(Rectangle{sw * 0.25f, sh - 50.f, sw * 0.5f, 12.f}, Color{40, 40, 50, 255});
    DrawRectangleRec(Rectangle{sw * 0.25f, sh - 50.f, sw * 0.5f * p, 12.f}, Color{220, 40, 70, 255});
  }

  bool Jailed() const { return jailed_; }

  void TryBribe(Character& p) {
    int cost = 20000 + static_cast<int>(jailTime_) * 100;
    if (p.Money() < cost) return;
    p.AddMoney(-cost);
    if (GetRandomValue(0, 100) < 55) Release(p);
  }

  void TryLawyer(Character& p) {
    int cost = 35000;
    if (p.Money() < cost) return;
    p.AddMoney(-cost);
    jailTime_ *= 0.5f;
  }

  void Release(Character& p) {
    jailed_ = false;
    jailTime_ = 0;
    article_ = CrimeArticle::None;
    workReduce_ = false;
    EventBus::Get().Emit(GameEvent::PlayerReleased, {});
    (void)p;
  }

  static CrimeArticle FromWantedAction(int wantedGain) {
    if (wantedGain >= 3) return CrimeArticle::Shooting;
    if (wantedGain >= 2) return CrimeArticle::CarTheft;
    return CrimeArticle::ResistArrest;
  }

private:
  const char* ArticleName() const {
    switch (article_) {
      case CrimeArticle::Drugs: return "Drugs";
      case CrimeArticle::IllegalWeapon: return "Illegal weapon";
      case CrimeArticle::CarTheft: return "Car theft";
      case CrimeArticle::Shooting: return "Shooting";
      case CrimeArticle::ResistArrest: return "Resist arrest";
      default: return "Unknown";
    }
  }

  bool jailed_ = false;
  float jailTime_ = 0.f;
  float jailMax_ = 0.f;
  bool workReduce_ = false;
  CrimeArticle article_ = CrimeArticle::None;
};
