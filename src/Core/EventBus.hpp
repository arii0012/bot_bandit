#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

enum class GameEvent {
  MoneyChanged,
  CarPurchased,
  HousePurchased,
  PoliceWantedChanged,
  PhoneHacked,
  PlayerArrested,
  PlayerReleased,
  Collision,
  HeatChanged,
  LevelUp,
  CutsceneEnded,
  Count
};

struct EventPayload {
  int64_t i0 = 0;
  int64_t i1 = 0;
  float f0 = 0.f;
  std::string s0;
};

class EventBus {
public:
  using Handler = std::function<void(const EventPayload&)>;

  static EventBus& Get() {
    static EventBus inst;
    return inst;
  }

  int Subscribe(GameEvent ev, Handler h) {
    int id = nextId_++;
    subs_[static_cast<int>(ev)].push_back({id, std::move(h)});
    return id;
  }

  void Unsubscribe(GameEvent ev, int id) {
    auto& v = subs_[static_cast<int>(ev)];
    v.erase(std::remove_if(v.begin(), v.end(),
                           [id](const Sub& s) { return s.id == id; }),
            v.end());
  }

  void Emit(GameEvent ev, const EventPayload& p = {}) {
    auto& v = subs_[static_cast<int>(ev)];
    // copy in case handlers unsubscribe
    auto snap = v;
    for (auto& s : snap) {
      if (s.fn) s.fn(p);
    }
  }

  void Clear() {
    for (auto& [_, v] : subs_) v.clear();
  }

private:
  struct Sub {
    int id;
    Handler fn;
  };
  std::unordered_map<int, std::vector<Sub>> subs_;
  int nextId_ = 1;
};
