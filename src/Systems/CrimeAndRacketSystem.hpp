#pragma once
#include "Character/Character.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Systems/ReputationScales.hpp"
#include "Core/FontUI.hpp"
#include <string>
#include <vector>
#include <algorithm>

struct Contact {
  const char* id;
  const char* name;
  int trust; // 0..100
};

class CrimeAndRacketSystem {
public:
  void AddContact(const char* id, const char* name) {
    for (auto& c : contacts_) if (c.id == id) return;
    contacts_.push_back({id, name, 10});
  }

  void Trust(const char* id, int d) {
    for (auto& c : contacts_)
      if (std::string(c.id) == id) {
        c.trust = std::max(0, std::min(100, c.trust + d));
        return;
      }
  }

  void ClaimPoint(const std::string& district) {
    points_.push_back(district);
    ReputationScales::Get().AddStreet(5);
  }

  int64_t CollectRacket() {
    int64_t sum = static_cast<int64_t>(points_.size()) * 15000LL;
    return sum;
  }

  void TickHotTimer(float dt) {
    if (hot_ > 0.f) hot_ = std::max(0.f, hot_ - dt);
  }

  void MarkHot(float seconds) { hot_ = std::max(hot_, seconds); }
  bool IsHot() const { return hot_ > 0.f; }
  float HotLeft() const { return hot_; }

  void AddConviction() {
    convictions_++;
    ReputationScales::Get().AddSystem(-15);
  }

  int Convictions() const { return convictions_; }
  bool CanTravelCapital() const { return convictions_ < 3; }

  void HackCamera(Character& p) {
    if (p.Money() < 20000) return;
    p.AddMoney(-20000);
    hot_ = std::max(0.f, hot_ - 40.f);
    Trust("hacker", 5);
  }

  void DrawHUD(float x, float y) {
    char buf[96];
    snprintf(buf, sizeof(buf), "Точки %d  |  Судимости %d  |  Жар %.0fs",
             (int)points_.size(), convictions_, hot_);
    FontUI::Get().Draw(buf, x, y, 13, IsHot() ? Color{255, 80, 80, 255} : Color{160, 160, 180, 255});
  }

private:
  std::vector<Contact> contacts_ = {
    {"hacker", "Hacker", 5},
    {"inform", "Informator", 5},
    {"inspector", "Inspector", 0},
    {"fence", "Skupshchik", 10},
  };
  std::vector<std::string> points_;
  float hot_ = 0.f;
  int convictions_ = 0;
};
