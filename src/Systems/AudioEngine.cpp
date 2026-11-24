#include "Systems/AudioEngine.hpp"
#include <cstdlib>

AudioEngine& AudioEngine::Get() {
  static AudioEngine inst;
  return inst;
}

Wave AudioEngine::MakeTone(float freq, float sec, float amp, bool noise) {
  const int sampleRate = 22050;
  int frames = static_cast<int>(sec * sampleRate);
  if (frames < 1) frames = 1;
  Wave w{};
  w.frameCount = frames;
  w.sampleRate = sampleRate;
  w.sampleSize = 16;
  w.channels = 1;
  short* data = static_cast<short*>(MemAlloc(frames * sizeof(short)));
  w.data = data;
  for (int i = 0; i < frames; ++i) {
    float t = static_cast<float>(i) / static_cast<float>(sampleRate);
    float env = 1.f;
    float fade = 0.02f;
    if (t < fade) env = t / fade;
    if (t > sec - fade) env = (sec - t) / fade;
    float s = 0.f;
    if (noise) {
      s = (static_cast<float>(GetRandomValue(0, 10000)) / 5000.f - 1.f) * amp;
    } else {
      s = sinf(2.f * PI * freq * t) * amp;
      s += sinf(2.f * PI * freq * 2.f * t) * amp * 0.25f;
    }
    s *= env;
    if (s > 1.f) s = 1.f;
    if (s < -1.f) s = -1.f;
    data[i] = static_cast<short>(s * 30000.f);
  }
  return w;
}

Wave AudioEngine::MakePhonkLoop(float sec) {
  const int sampleRate = 22050;
  int frames = static_cast<int>(sec * sampleRate);
  Wave w{};
  w.frameCount = frames;
  w.sampleRate = sampleRate;
  w.sampleSize = 16;
  w.channels = 1;
  short* data = static_cast<short*>(MemAlloc(frames * sizeof(short)));
  w.data = data;
  for (int i = 0; i < frames; ++i) {
    float t = static_cast<float>(i) / static_cast<float>(sampleRate);
    float beat = fmodf(t * 2.2f, 1.f);
    float kick = expf(-beat * 18.f) * 0.55f;
    float bass = sinf(2.f * PI * 55.f * t) * 0.28f * (0.6f + 0.4f * sinf(t * 3.f));
    float lead = sinf(2.f * PI * 110.f * t + sinf(t * 8.f)) * 0.12f;
    float s = kick * sinf(2.f * PI * 70.f * t) + bass + lead;
    if (s > 1.f) s = 1.f;
    if (s < -1.f) s = -1.f;
    data[i] = static_cast<short>(s * 28000.f);
  }
  return w;
}

Wave AudioEngine::MakeLofiLoop(float sec) {
  const int sampleRate = 22050;
  int frames = static_cast<int>(sec * sampleRate);
  Wave w{};
  w.frameCount = frames;
  w.sampleRate = sampleRate;
  w.sampleSize = 16;
  w.channels = 1;
  short* data = static_cast<short*>(MemAlloc(frames * sizeof(short)));
  w.data = data;
  for (int i = 0; i < frames; ++i) {
    float t = static_cast<float>(i) / static_cast<float>(sampleRate);
    float pad = sinf(2.f * PI * 130.f * t) * 0.08f + sinf(2.f * PI * 164.f * t) * 0.06f;
    float soft = sinf(2.f * PI * 98.f * t) * 0.1f;
    float noise = (static_cast<float>(GetRandomValue(0, 1000)) / 1000.f - 0.5f) * 0.02f;
    float s = pad + soft + noise;
    data[i] = static_cast<short>(s * 26000.f);
  }
  return w;
}

Wave AudioEngine::MakeActionLoop(float sec) {
  const int sampleRate = 22050;
  int frames = static_cast<int>(sec * sampleRate);
  Wave w{};
  w.frameCount = frames;
  w.sampleRate = sampleRate;
  w.sampleSize = 16;
  w.channels = 1;
  short* data = static_cast<short*>(MemAlloc(frames * sizeof(short)));
  w.data = data;
  for (int i = 0; i < frames; ++i) {
    float t = static_cast<float>(i) / static_cast<float>(sampleRate);
    float beat = fmodf(t * 3.4f, 1.f);
    float kick = expf(-beat * 22.f) * 0.7f;
    float snare = (fmodf(t * 3.4f + 0.5f, 1.f) < 0.08f) ? 0.35f : 0.f;
    float bass = sinf(2.f * PI * 80.f * t) * 0.3f;
    float saw = (fmodf(t * 180.f, 1.f) * 2.f - 1.f) * 0.08f;
    float s = kick * sinf(2.f * PI * 90.f * t) + snare * (GetRandomValue(0, 100) / 100.f * 0.4f) + bass + saw;
    if (s > 1.f) s = 1.f;
    if (s < -1.f) s = -1.f;
    data[i] = static_cast<short>(s * 29000.f);
  }
  return w;
}

Wave AudioEngine::MakeClubLoop(float sec) {
  const int sampleRate = 22050;
  int frames = static_cast<int>(sec * sampleRate);
  Wave w{};
  w.frameCount = frames;
  w.sampleRate = sampleRate;
  w.sampleSize = 16;
  w.channels = 1;
  short* data = static_cast<short*>(MemAlloc(frames * sizeof(short)));
  w.data = data;
  for (int i = 0; i < frames; ++i) {
    float t = static_cast<float>(i) / static_cast<float>(sampleRate);
    float beat = fmodf(t * 2.0f, 1.f);
    float kick = expf(-beat * 14.f) * 0.65f;
    float sub = sinf(2.f * PI * 45.f * t) * 0.4f;
    float hat = (fmodf(t * 8.f, 1.f) < 0.05f) ? 0.12f : 0.f;
    float s = kick * sinf(2.f * PI * 60.f * t) + sub + hat * (GetRandomValue(0, 100) / 100.f);
    if (s > 1.f) s = 1.f;
    if (s < -1.f) s = -1.f;
    data[i] = static_cast<short>(s * 30000.f);
  }
  return w;
}

Sound AudioEngine::LoadFromWaveOwned(Wave w) {
  Sound s = LoadSoundFromWave(w);
  UnloadWave(w);
  return s;
}

void AudioEngine::Init() {
  if (ready_) return;
  InitAudioDevice();
  if (!IsAudioDeviceReady()) {
    TraceLog(LOG_WARNING, "AudioEngine: device not ready");
    return;
  }

  music_[0] = LoadFromWaveOwned(MakePhonkLoop(4.f));
  music_[1] = LoadFromWaveOwned(MakeLofiLoop(5.f));
  music_[2] = LoadFromWaveOwned(MakeActionLoop(3.5f));
  music_[3] = LoadFromWaveOwned(MakeClubLoop(4.f));
  for (int i = 0; i < 4; ++i) musicValid_[i] = music_[i].frameCount > 0;

  sfx_[(int)SfxId::Click] = LoadFromWaveOwned(MakeTone(880.f, 0.05f, 0.3f, false));
  sfx_[(int)SfxId::Hover] = LoadFromWaveOwned(MakeTone(660.f, 0.03f, 0.15f, false));
  sfx_[(int)SfxId::Coin] = LoadFromWaveOwned(MakeTone(1200.f, 0.12f, 0.35f, false));
  sfx_[(int)SfxId::Notify] = LoadFromWaveOwned(MakeTone(520.f, 0.18f, 0.25f, false));
  sfx_[(int)SfxId::TireScreech] = LoadFromWaveOwned(MakeTone(400.f, 0.35f, 0.2f, true));
  sfx_[(int)SfxId::BlowOff] = LoadFromWaveOwned(MakeTone(200.f, 0.15f, 0.35f, true));
  sfx_[(int)SfxId::Gunshot] = LoadFromWaveOwned(MakeTone(120.f, 0.1f, 0.55f, true));
  for (int i = 0; i < (int)SfxId::Count; ++i) {
    if (i == (int)SfxId::Engine) continue;
    sfxValid_[i] = sfx_[i].frameCount > 0;
  }

  engine_ = LoadFromWaveOwned(MakeTone(90.f, 0.5f, 0.4f, false));
  engineValid_ = engine_.frameCount > 0;

  // optional file override
  if (FileExists("assets/audio/bgm.wav")) {
    Sound file = LoadSound("assets/audio/bgm.wav");
    if (file.frameCount > 0) {
      if (musicValid_[0]) UnloadSound(music_[0]);
      music_[0] = file;
      musicValid_[0] = true;
    }
  }

  ready_ = true;
  SetMood(MusicMood::Menu);
}

void AudioEngine::Shutdown() {
  if (!ready_) return;
  for (int i = 0; i < 4; ++i) {
    if (musicValid_[i]) UnloadSound(music_[i]);
    musicValid_[i] = false;
  }
  for (int i = 0; i < (int)SfxId::Count; ++i) {
    if (sfxValid_[i]) UnloadSound(sfx_[i]);
    sfxValid_[i] = false;
  }
  if (engineValid_) UnloadSound(engine_);
  engineValid_ = false;
  CloseAudioDevice();
  ready_ = false;
}

void AudioEngine::SetMasterVolume(float v) {
  master_ = std::clamp(v, 0.f, 1.f);
}

void AudioEngine::SetMusicVolume(float v) { musicVol_ = std::clamp(v, 0.f, 1.f); }
void AudioEngine::SetSfxVolume(float v) { sfxVol_ = std::clamp(v, 0.f, 1.f); }

void AudioEngine::SetMood(MusicMood mood) {
  if (mood == targetMood_) return;
  targetMood_ = mood;
  cross_ = 0.f;
}

void AudioEngine::Update(float dt) {
  if (!ready_) return;
  if (dt > 0.1f) dt = 0.1f;
  cross_ += dt * 0.8f;
  if (cross_ >= 1.f) {
    cross_ = 1.f;
    mood_ = targetMood_;
  }

  int idx = -1;
  switch (targetMood_) {
    case MusicMood::Menu: idx = 0; break;
    case MusicMood::City: idx = 1; break;
    case MusicMood::Chase: idx = 2; break;
    case MusicMood::Club: idx = 3; break;
    default: idx = -1; break;
  }

  if (idx >= 0 && musicValid_[idx]) {
    if (musicPlaying_ != idx) {
      if (musicPlaying_ >= 0 && musicValid_[musicPlaying_]) StopSound(music_[musicPlaying_]);
      musicPlaying_ = idx;
      PlaySound(music_[idx]);
    }
    if (!IsSoundPlaying(music_[idx])) PlaySound(music_[idx]);
    SetSoundVolume(music_[idx], musicVol_ * master_ * cross_);
  } else if (musicPlaying_ >= 0 && musicValid_[musicPlaying_]) {
    StopSound(music_[musicPlaying_]);
    musicPlaying_ = -1;
  }

  if (engineValid_ && engineVol_ > 0.01f) {
    if (!IsSoundPlaying(engine_)) PlaySound(engine_);
    SetSoundVolume(engine_, engineVol_ * sfxVol_ * master_ * 0.4f);
  } else if (engineValid_) {
    StopSound(engine_);
  }
}

void AudioEngine::PlaySfx(SfxId id, float pitch, float vol) {
  if (!ready_) return;
  int i = static_cast<int>(id);
  if (i < 0 || i >= (int)SfxId::Count || !sfxValid_[i]) return;
  SetSoundPitch(sfx_[i], pitch);
  SetSoundVolume(sfx_[i], vol * sfxVol_ * master_);
  PlaySound(sfx_[i]);
}

void AudioEngine::SetEngine(float rpm01, bool drifting, bool nitro) {
  if (!ready_ || !engineValid_) return;
  if (rpm01 < 0.f) rpm01 = 0.f;
  if (rpm01 > 1.f) rpm01 = 1.f;
  engineVol_ = rpm01;
  SetSoundPitch(engine_, 0.7f + rpm01 * 0.9f + (nitro ? 0.15f : 0.f));
  if (drifting && rpm01 > 0.2f) PlaySfx(SfxId::TireScreech, 0.9f + rpm01 * 0.2f, 0.35f);
  if (nitro) PlaySfx(SfxId::BlowOff, 1.f, 0.4f);
}
