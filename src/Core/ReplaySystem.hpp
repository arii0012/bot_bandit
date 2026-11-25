#pragma once
#include "raylib.h"
#include <vector>
#include <cmath>

struct ReplayFrame {
  float x, y, facing, speed;
  int wanted;
  float t;
};

class ReplaySystem {
public:
  static constexpr int kMaxFrames = 900; // ~15s @ 60fps

  void Push(float x, float y, float facing, float speed, int wanted, float t) {
    if ((int)buf_.size() >= kMaxFrames) {
      buf_.erase(buf_.begin());
    }
    buf_.push_back({x, y, facing, speed, wanted, t});
  }

  void TogglePlayback() {
    playing_ = !playing_;
    if (playing_) {
      idx_ = 0;
      slowMo_ = true;
    }
  }

  bool Playing() const { return playing_; }

  void Update(float dt) {
    if (!playing_ || buf_.empty()) return;
    float step = slowMo_ ? dt * 0.35f : dt;
    acc_ += step * 60.f;
    while (acc_ >= 1.f && idx_ < (int)buf_.size() - 1) {
      acc_ -= 1.f;
      idx_++;
    }
    if (idx_ >= (int)buf_.size() - 1) {
      playing_ = false;
      idx_ = 0;
    }
  }

  void Draw(int sw, int sh) {
    if (!playing_ || buf_.empty()) return;
    DrawRectangle(0, 0, sw, 28, Color{0, 0, 0, 180});
    DrawText("REPLAY F9  (slow-mo)", 16, 6, 16, Color{0, 220, 255, 255});
    const auto& f = buf_[idx_];
    // trail
    int from = idx_ > 40 ? idx_ - 40 : 0;
    for (int i = from; i < idx_; ++i) {
      float a = (float)(i - from) / 40.f;
      DrawCircle(static_cast<int>(buf_[i].x), static_cast<int>(buf_[i].y), 3.f,
                 Fade(Color{255, 60, 90, 255}, a * 0.6f));
    }
    DrawCircle(static_cast<int>(f.x), static_cast<int>(f.y), 8, Color{0, 220, 255, 255});
  }

  void Clear() { buf_.clear(); idx_ = 0; playing_ = false; }

private:
  std::vector<ReplayFrame> buf_;
  int idx_ = 0;
  float acc_ = 0.f;
  bool playing_ = false;
  bool slowMo_ = true;
};
