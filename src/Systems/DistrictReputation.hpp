#pragma once
#include <string>
#include <unordered_map>
#include <algorithm>

enum class DistrictStanding { Nobody, Hero, Rat };

class DistrictReputation {
public:
  static DistrictReputation& Get() {
    static DistrictReputation inst;
    return inst;
  }

  void Add(const std::string& district, int delta) {
    score_[district] = std::max(-50, std::min(50, score_[district] + delta));
  }

  DistrictStanding Standing(const std::string& district) const {
    auto it = score_.find(district);
    int s = it == score_.end() ? 0 : it->second;
    if (s >= 15) return DistrictStanding::Hero;
    if (s <= -15) return DistrictStanding::Rat;
    return DistrictStanding::Nobody;
  }

  float PriceMult(const std::string& district) const {
    switch (Standing(district)) {
      case DistrictStanding::Hero: return 0.9f;
      case DistrictStanding::Rat: return 1.25f;
      default: return 1.f;
    }
  }

private:
  std::unordered_map<std::string, int> score_;
};
