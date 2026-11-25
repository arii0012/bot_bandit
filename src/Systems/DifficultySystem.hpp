#pragma once
#include "Character/Character.hpp"
#include <string>

enum class Difficulty : int {
  Newbie = 0,   // Novice
  Street = 1,   // Balanced
  Hardcore = 2, // Realism
  Nineties = 3  // Extreme 90s
};

class DifficultySystem {
public:
  static DifficultySystem& Get() {
    static DifficultySystem inst;
    return inst;
  }

  void Set(Difficulty d) { diff_ = d; }
  Difficulty GetDiff() const { return diff_; }

  const char* Name() const {
    switch (diff_) {
      case Difficulty::Newbie: return "Novichok";
      case Difficulty::Street: return "Ulichnyy";
      case Difficulty::Hardcore: return "Hardcore";
      case Difficulty::Nineties: return "90-e Vyzhivanie";
      default: return "Ulichnyy";
    }
  }

  int64_t StartingMoney() const {
    switch (diff_) {
      case Difficulty::Newbie: return 150000;
      case Difficulty::Street: return 45000;
      case Difficulty::Hardcore: return 12000;
      case Difficulty::Nineties: return 4000;
      default: return 45000;
    }
  }

  float QuestPayMult() const {
    switch (diff_) {
      case Difficulty::Newbie: return 1.35f;
      case Difficulty::Street: return 1.0f;
      case Difficulty::Hardcore: return 0.9f;
      case Difficulty::Nineties: return 0.85f;
      default: return 1.f;
    }
  }

  float SmugglePayMult() const {
    return diff_ == Difficulty::Nineties ? 2.0f : 1.0f;
  }

  float PoliceAggro() const {
    switch (diff_) {
      case Difficulty::Newbie: return 0.55f;
      case Difficulty::Street: return 1.0f;
      case Difficulty::Hardcore: return 1.45f;
      case Difficulty::Nineties: return 1.7f;
      default: return 1.f;
    }
  }

  float RepairCostMult() const {
    switch (diff_) {
      case Difficulty::Newbie: return 0.6f;
      case Difficulty::Street: return 1.0f;
      case Difficulty::Hardcore: return 1.4f;
      case Difficulty::Nineties: return 1.8f;
      default: return 1.f;
    }
  }

  float FuelDrainMult() const {
    switch (diff_) {
      case Difficulty::Newbie: return 0.7f;
      case Difficulty::Street: return 1.0f;
      case Difficulty::Hardcore: return 1.5f;
      case Difficulty::Nineties: return 1.8f;
      default: return 1.f;
    }
  }

  bool FullConfiscationOnArrest() const {
    return diff_ == Difficulty::Hardcore || diff_ == Difficulty::Nineties;
  }

  void ApplyNewGame(Character& p) {
    p.money_ = StartingMoney();
    p.Inv().cars.clear();
    p.activeCar_.clear();
    p.SetWanted(0);
    p.heat_ = 0;
    p.energy_ = 100;
    p.level_ = 1;
    p.exp_ = 0;
  }

private:
  Difficulty diff_ = Difficulty::Street;
};
