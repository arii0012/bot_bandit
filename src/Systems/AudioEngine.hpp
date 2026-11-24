#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <algorithm>

enum class MusicMood {
  Menu,
  City,
  Chase,
  Club,
  Silent
};

enum class SfxId {
  Click,
  Hover,
  Coin,
  Notify,
  Engine,
  TireScreech,
  BlowOff,
  Gunshot,
  Count
};

class AudioEngine {
public:
  static AudioEngine& Get();

  void Init();
  void Shutdown();
  void Update(float dt);

  void SetMood(MusicMood mood);
  void SetMasterVolume(float v);
  void SetMusicVolume(float v);
  void SetSfxVolume(float v);

  void PlaySfx(SfxId id, float pitch = 1.f, float vol = 1.f);
  void SetEngine(float rpm01, bool drifting, bool nitro);

  MusicMood Mood() const { return mood_; }

private:
  AudioEngine() = default;

  Wave MakeTone(float freq, float sec, float amp, bool noise);
  Wave MakePhonkLoop(float sec);
  Wave MakeLofiLoop(float sec);
  Wave MakeActionLoop(float sec);
  Wave MakeClubLoop(float sec);
  Sound LoadFromWaveOwned(Wave w);

  bool ready_ = false;
  MusicMood mood_ = MusicMood::Silent;
  MusicMood targetMood_ = MusicMood::Silent;
  float cross_ = 1.f;

  Sound music_[4]{};
  bool musicValid_[4]{false, false, false, false};
  int musicPlaying_ = -1;

  Sound sfx_[(int)SfxId::Count]{};
  bool sfxValid_[(int)SfxId::Count]{};

  Sound engine_;
  bool engineValid_ = false;
  float engineVol_ = 0.f;

  float master_ = 1.f;
  float musicVol_ = 0.7f;
  float sfxVol_ = 0.9f;
};
