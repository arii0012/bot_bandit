#pragma once
#include <algorithm>

class ReputationScales {
public:
  static ReputationScales& Get() {
    static ReputationScales inst;
    return inst;
  }

  void AddStreet(int v) { street_ = Clamp(street_ + v); }
  void AddSystem(int v) { system_ = Clamp(system_ + v); }
  void AddDebt(int v) { debt_ = Clamp(debt_ + v); }

  int Street() const { return street_; }
  int System() const { return system_; }
  int Debt() const { return debt_; }

  void Reset() { street_ = system_ = debt_ = 0; }

  // finale helpers
  bool IsMagnate() const { return system_ >= 40 && debt_ >= -20; }
  bool IsStreetKing() const { return street_ >= 40; }
  bool IsCrimeBoss() const { return street_ >= 30 && system_ <= -20; }
  bool IsZeroAgain() const { return street_ <= -30 && system_ <= -30 && debt_ <= -30; }

private:
  static int Clamp(int v) { return std::max(-100, std::min(100, v)); }
  int street_ = 0;
  int system_ = 0;
  int debt_ = 0;
};
