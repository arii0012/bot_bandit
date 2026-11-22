#include "EngineAudio.hpp"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>

EngineAudio* EngineAudio::inst_ = nullptr;

EngineAudio::EngineAudio() {
  inst_ = this;
  if (!IsAudioDeviceReady()) InitAudioDevice();
  if (IsAudioDeviceReady()) {
    // smaller buffer / lower rate = less CPU
    stream_ = LoadAudioStream(22050, 16, 1);
    SetAudioStreamBufferSizeDefault(1024);
    SetAudioStreamCallback(stream_, Callback);
    ready_ = true;
  }
}

EngineAudio::~EngineAudio() {
  Stop();
  if (ready_) UnloadAudioStream(stream_);
  inst_ = nullptr;
}

void EngineAudio::Start() {
  if (!ready_ || playing_) return;
  PlayAudioStream(stream_);
  playing_ = true;
}

void EngineAudio::Stop() {
  if (!ready_ || !playing_) return;
  StopAudioStream(stream_);
  playing_ = false;
  phase_ = 0.f;
}

void EngineAudio::Update(float speedAbs, bool drifting, bool nitro) {
  if (!ready_) return;
  float target = 700.f + speedAbs * 2.0f + (drifting ? 280.f : 0.f) + (nitro ? 450.f : 0.f);
  // critically damped-ish smoothing
  rpm_ += (target - rpm_) * 0.12f;
  rpm_ = std::clamp(rpm_, 600.f, 6500.f);

  if (!playing_ && speedAbs > 8.f) Start();
  if (playing_ && speedAbs < 2.f && !nitro) Stop();
}

void EngineAudio::Callback(void* buffer, unsigned int frames) {
  auto* self = inst_;
  short* out = static_cast<short*>(buffer);
  if (!self || !self->playing_) {
    std::memset(buffer, 0, frames * sizeof(short));
    return;
  }
  // precompute
  const float rpm = self->rpm_;
  const float invSr = 1.f / 22050.f;
  float freq = 36.f + rpm * 0.038f;
  float vol = 0.18f + (rpm / 6500.f) * 0.32f;
  vol = std::min(vol, 0.48f);
  float phase = self->phase_;
  // cheap LCG noise instead of rand()
  static unsigned n = 1u;
  for (unsigned int i = 0; i < frames; ++i) {
    phase += freq * invSr;
    if (phase >= 1.f) phase -= 1.f;
    // band-limited-ish: 2 harmonics + soft square
    float twopi = phase * 6.2831853f;
    float wave = sinf(twopi) * 0.55f + sinf(twopi * 2.f) * 0.22f + sinf(twopi * 3.f) * 0.08f;
    n = n * 1664525u + 1013904223u;
    float noise = ((n >> 16) & 0x7fff) / 32768.f - 0.5f;
    wave += noise * 0.06f;
    out[i] = (short)(wave * vol * 28000.f);
  }
  self->phase_ = phase;
}
