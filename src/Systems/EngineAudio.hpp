#pragma once
#include "raylib.h"

class EngineAudio {
public:
  EngineAudio();
  ~EngineAudio();
  void Start();
  void Stop();
  void Update(float speedAbs, bool drifting, bool nitro);
  bool Playing() const { return playing_; }

private:
  AudioStream stream_{};
  bool playing_ = false;
  bool ready_ = false;
  float phase_ = 0.f;
  float rpm_ = 800.f;
  static void Callback(void* buffer, unsigned int frames);
  static EngineAudio* inst_;
};
