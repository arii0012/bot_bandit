#pragma once
#include "Character/Character.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Core/FontUI.hpp"
#include "raylib.h"
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdio>

class VehicleDeepSystem {
public:
  void OnCarBought() {
    owned_ = true;
    dirt_ = 0.f;
    hideLevel_ = 0;
    ecuStage_ = 0;
    insured_ = false;
    hp_ = 100.f;
  }

  bool HasCar() const { return owned_; }
  int EcuStage() const { return ecuStage_; }
  float Dirt() const { return dirt_; }
  int HideLevel() const { return hideLevel_; }
  bool Insured() const { return insured_; }

  void ChipTune(Character& p, int stage) {
    if (!owned_ || stage < 1 || stage > 3) return;
    int64_t cost = 80000LL * stage;
    if (p.Money() < cost) return;
    p.AddMoney(-cost);
    ecuStage_ = stage;
    wearMult_ = 1.f + stage * 0.25f;
  }

  void InstallHide(Character& p) {
    if (!owned_ || hideLevel_ >= 3) return;
    int64_t cost = 120000LL * (hideLevel_ + 1);
    if (p.Money() < cost) return;
    p.AddMoney(-cost);
    hideLevel_++;
  }

  void BuyInsurance(Character& p) {
    if (p.Money() < 45000) return;
    p.AddMoney(-45000);
    insured_ = true;
  }

  void Wash(Character& p) {
    if (p.Money() < 3000) return;
    p.AddMoney(-3000);
    dirt_ = 0.f;
  }

  void TickDrive(float dt, bool raining) {
    if (!owned_) return;
    dirt_ = std::min(1.f, dirt_ + dt * (raining ? 0.02f : 0.005f));
    hp_ = std::max(0.f, hp_ - dt * 0.01f * wearMult_);
  }

  // 0..1 chance to find contraband on search
  float SearchDetectChance() const {
    float base = 0.55f - hideLevel_ * 0.12f;
    return std::max(0.1f, base);
  }

  int DynoHP() const {
    return 90 + ecuStage_ * 35 + static_cast<int>((100.f - dirt_ * 20.f));
  }

  void Impound(Character& p) {
    impounded_ = true;
    impoundFee_ = insured_ ? 25000 : 80000;
    (void)p;
  }

  bool TryRecover(Character& p) {
    if (!impounded_) return true;
    if (p.Money() < impoundFee_) return false;
    p.AddMoney(-impoundFee_);
    impounded_ = false;
    return true;
  }

  void DrawGarageStats(float x, float y) {
    if (!owned_) {
      FontUI::Get().Draw("Нет автомобиля — купи ВАЗ или Приору", x, y, 16, Color{180, 180, 200, 255});
      return;
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "Этап ЭБУ %d  |  %d л.с.  |  Скрытие %d  |  Грязь %.0f%%",
             ecuStage_, DynoHP(), hideLevel_, dirt_ * 100.f);
    FontUI::Get().Draw(buf, x, y, 14, WHITE);
    snprintf(buf, sizeof(buf), "Страховка: %s  |  Прочность кузова %.0f",
             insured_ ? "да" : "нет", hp_);
    FontUI::Get().Draw(buf, x, y + 22, 14, Color{200, 200, 160, 255});
  }

private:
  bool owned_ = false;
  bool insured_ = false;
  bool impounded_ = false;
  int ecuStage_ = 0;
  int hideLevel_ = 0;
  int64_t impoundFee_ = 0;
  float dirt_ = 0.f;
  float hp_ = 100.f;
  float wearMult_ = 1.f;
};
