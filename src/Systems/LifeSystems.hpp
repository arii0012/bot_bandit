#pragma once
#include "Character/Character.hpp"
#include "Systems/ReputationScales.hpp"
#include "Systems/NPCMemorySystem.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Core/FontUI.hpp"
#include "raylib.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>

class HousingSafe {
public:
  void OnHouseBought(int tier) {
    tier_ = std::max(tier_, tier);
    safeCap_ = 50000LL * tier_;
  }
  int Tier() const { return tier_; }
  int64_t SafeCap() const { return safeCap_; }
  int64_t SafeMoney() const { return safeMoney_; }

  bool Deposit(Character& p, int64_t amount) {
    if (amount <= 0 || p.Money() < amount) return false;
    int64_t room = safeCap_ - safeMoney_;
    if (room <= 0) return false;
    int64_t put = std::min(amount, room);
    p.AddMoney(-put);
    safeMoney_ += put;
    return true;
  }

  bool Withdraw(Character& p, int64_t amount) {
    if (amount <= 0 || safeMoney_ < amount) return false;
    safeMoney_ -= amount;
    p.AddMoney(amount);
    return true;
  }

  // confiscation skips safe
  float RestEnergyBonus() const { return tier_ > 0 ? 1.5f : 1.f; }

private:
  int tier_ = 0;
  int64_t safeCap_ = 0;
  int64_t safeMoney_ = 0;
};

class RadioSystem {
public:
  void NextWave() { wave_ = (wave_ + 1) % 4; }
  int Wave() const { return wave_; }
  const char* WaveName() const {
    const char* n[] = {"Phonk FM", "DPS Alert", "Band Chat", "Night Lo-Fi"};
    return n[wave_ % 4];
  }
  const char* Blurb() const {
    const char* b[] = {
      "Bass. Gorod ne spit.",
      "Post na yuge. Snizhay skorost'.",
      "Zakaz: peregon. Oplata nalom.",
      "Dozhd'. Spokoynaya volna."};
    return b[wave_ % 4];
  }
private:
  int wave_ = 0;
};

class FamilyCalls {
public:
  void Update(float dt, Character& p) {
    timer_ -= dt;
    if (timer_ > 0) return;
    timer_ = 180.f + static_cast<float>(GetRandomValue(0, 120));
    pending_ = true;
    ask_ = 5000 + GetRandomValue(0, 25000);
  }

  bool Pending() const { return pending_; }
  int64_t AskAmount() const { return ask_; }

  void Accept(Character& p) {
    if (!pending_) return;
    if (p.Money() >= ask_) {
      p.AddMoney(-ask_);
      ReputationScales::Get().AddDebt(8);
      NPCMemorySystem::Get().Remember("family", "helped", 1);
    }
    pending_ = false;
  }

  void Refuse() {
    if (!pending_) return;
    ReputationScales::Get().AddDebt(-12);
    NPCMemorySystem::Get().Remember("family", "refused", 1);
    pending_ = false;
  }

  void Draw(int sw) {
    if (!pending_) return;
    float w = 420.f, h = 70.f;
    float x = (sw - w) * 0.5f;
    DrawRectangleRounded(Rectangle{x, 50, w, h}, 0.2f, 10, Color{20, 20, 32, 240});
    FontUI::Get().DrawCentered("ZVONOK: nuzhny den'gi", sw * 0.5f, 58, 16, WHITE);
    std::string s = ArtworkRenderer::FormatRub(ask_) + "  [Y] dat'  [N] otkaz";
    FontUI::Get().DrawCentered(s.c_str(), sw * 0.5f, 88, 14, Color{244, 196, 48, 255});
  }

private:
  float timer_ = 90.f;
  bool pending_ = false;
  int64_t ask_ = 0;
};

class FinaleSystem {
public:
  enum class Ending { None, Magnate, StreetKing, CrimeBoss, ZeroAgain };

  Ending Evaluate() const {
    auto& r = ReputationScales::Get();
    if (r.IsZeroAgain()) return Ending::ZeroAgain;
    if (r.IsMagnate()) return Ending::Magnate;
    if (r.IsCrimeBoss()) return Ending::CrimeBoss;
    if (r.IsStreetKing()) return Ending::StreetKing;
    return Ending::None;
  }

  const char* Nickname() const {
    auto& r = ReputationScales::Get();
    if (r.Street() >= 40) return "Korol'";
    if (r.System() >= 40) return "Magnat";
    if (r.Debt() <= -30) return "Krysa";
    if (r.Street() <= -20) return "Nol'";
    return "Brat";
  }

  void DrawEnding(int sw, int sh) {
    Ending e = Evaluate();
    if (e == Ending::None) return;
    DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 220});
    const char* title = "FINAL";
    if (e == Ending::Magnate) title = "LEGAL'NYY MAGNAT";
    if (e == Ending::StreetKing) title = "KOROL' ULITS";
    if (e == Ending::CrimeBoss) title = "KRIMINAL'NYY BOSS";
    if (e == Ending::ZeroAgain) title = "NOL' SNOVA - NG+";
    FontUI::Get().DrawCentered(title, sw * 0.5f, sh * 0.4f, 32, Color{0, 220, 255, 255});
    std::string nick = std::string("Tebya zvali: ") + Nickname();
    FontUI::Get().DrawCentered(nick.c_str(), sw * 0.5f, sh * 0.5f, 22, WHITE);
  }
};
