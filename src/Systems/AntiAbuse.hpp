#pragma once
#include <string>
#include <unordered_map>
#include <cmath>

class AntiAbuse {
public:
  static AntiAbuse& Get() {
    static AntiAbuse inst;
    return inst;
  }

  // returns true if action allowed
  bool TryAction(const std::string& key, float cooldownSec, float now) {
    auto it = last_.find(key);
    if (it != last_.end() && now - it->second < cooldownSec) return false;
    last_[key] = now;
    return true;
  }

  float Remaining(const std::string& key, float cooldownSec, float now) const {
    auto it = last_.find(key);
    if (it == last_.end()) return 0.f;
    float left = cooldownSec - static_cast<float>(now - it->second);
    return left > 0.f ? left : 0.f;
  }

  // Instagram-style post: 12 game hours = 12 real seconds * scale; use 12.f * 60 for longer
  static constexpr float kInstaCooldown = 12.f * 60.f; // 12 min real as stand-in for 12 game hours

  bool TryInstaPost(float now, const std::string& photoHash) {
    if (photoHash == lastPhoto_ && !lastPhoto_.empty()) {
      // same photo -> 0 payout path
      return false;
    }
    if (!TryAction("insta_post", kInstaCooldown, now)) return false;
    lastPhoto_ = photoHash;
    return true;
  }

  bool SamePhotoBlocked(const std::string& photoHash) const {
    return !lastPhoto_.empty() && photoHash == lastPhoto_;
  }

private:
  std::unordered_map<std::string, float> last_;
  std::string lastPhoto_;
};
