#pragma once
#include "raylib.h"
#include <string>
#include <cmath>
#include <cstring>
#include <algorithm>

enum class MusicMood {
  Menu = 0,
  City,
  Chase,
  Club,
  Tension,
  RainAlley,
  Victory,
  Silent
};

enum class RadioStation {
  StreetFM = 0,
  RetroWave,
  CrimeChannel,
  NightJazz,
  Count
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
  Footstep,
  Ignition,
  Siren,
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

  void SetRadio(RadioStation st);
  void NextRadio();
  RadioStation Radio() const { return radio_; }
  const char* RadioName() const;

  MusicMood Mood() const { return mood_; }

private:
  AudioEngine() = default;

  Wave MakeTone(float freq, float sec, float amp, bool noise);
  Wave MakePhonkLoop(float sec);
  Wave MakeLofiLoop(float sec);
  Wave MakeActionLoop(float sec);
  Wave MakeClubLoop(float sec);
  Wave MakeTensionLoop(float sec);
  Wave MakeRainLoop(float sec);
  Wave MakeVictorySting(float sec);
  Sound LoadFromWaveOwned(Wave w);

  bool ready_ = false;
  MusicMood mood_ = MusicMood::Silent;
  MusicMood targetMood_ = MusicMood::Silent;
  float cross_ = 1.f;

  static constexpr int kTracks = 7;
  Sound music_[kTracks]{};
  bool musicValid_[kTracks]{};
  int musicPlaying_ = -1;

  Sound sfx_[(int)SfxId::Count]{};
  bool sfxValid_[(int)SfxId::Count]{};

  Sound engine_;
  bool engineValid_ = false;
  float engineVol_ = 0.f;

  RadioStation radio_ = RadioStation::StreetFM;
  float radioVol_ = 0.5f;

  float master_ = 1.f;
  float musicVol_ = 0.7f;
  float sfxVol_ = 0.9f;
};
