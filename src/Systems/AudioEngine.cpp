#include "Systems/AudioEngine.hpp"
#include <cstdlib>
#include <vector>

AudioEngine& AudioEngine::Get() {
  static AudioEngine inst;
  return inst;
}

Wave AudioEngine::MakeTone(float freq, float sec, float amp, bool noise) {
  const int sr = 22050;
  int n = static_cast<int>(sec * sr);
  Wave w{};
  w.frameCount = n;
  w.sampleRate = sr;
  w.sampleSize = 16;
  w.channels = 1;
  auto* data = (short*)MemAlloc(n * sizeof(short));
  for (int i = 0; i < n; ++i) {
    float t = static_cast<float>(i) / sr;
    float s = noise ? ((rand() % 2000) / 1000.f - 1.f) * amp * 0.3f
                    : sinf(2.f * 3.14159265f * freq * t) * amp;
    float env = 1.f;
    if (i < sr / 20) env = i / (sr / 20.f);
    if (i > n - sr / 10) env = (n - i) / (sr / 10.f);
    data[i] = static_cast<short>(std::max(-32767.f, std::min(32767.f, s * env * 32767.f)));
  }
  w.data = data;
  return w;
}

Wave AudioEngine::MakePhonkLoop(float sec) {
  const int sr = 22050;
  int n = static_cast<int>(sec * sr);
  Wave w{};
  w.frameCount = n; w.sampleRate = sr; w.sampleSize = 16; w.channels = 1;
  auto* d = (short*)MemAlloc(n * sizeof(short));
  for (int i = 0; i < n; ++i) {
    float t = (float)i / sr;
    float kick = (fmodf(t, 0.5f) < 0.05f) ? sinf(2*3.14159f*60*t) * expf(-fmodf(t,0.5f)*20.f) : 0.f;
    float bass = sinf(2*3.14159f*55.f*t) * 0.25f + sinf(2*3.14159f*82.5f*t) * 0.12f;
    float hat = (fmodf(t, 0.25f) < 0.02f) ? ((rand()%1000)/1000.f - 0.5f) * 0.2f : 0.f;
    float s = (kick * 0.8f + bass + hat) * 0.5f;
    d[i] = (short)(std::max(-1.f, std::min(1.f, s)) * 30000);
  }
  w.data = d;
  return w;
}

Wave AudioEngine::MakeLofiLoop(float sec) {
  const int sr = 22050;
  int n = static_cast<int>(sec * sr);
  Wave w{};
  w.frameCount = n; w.sampleRate = sr; w.sampleSize = 16; w.channels = 1;
  auto* d = (short*)MemAlloc(n * sizeof(short));
  for (int i = 0; i < n; ++i) {
    float t = (float)i / sr;
    float s = sinf(2*3.14159f*220*t) * 0.15f + sinf(2*3.14159f*330*t) * 0.1f;
    s += ((rand()%100)/100.f - 0.5f) * 0.03f;
    s *= 0.6f + 0.2f * sinf(t * 0.5f);
    d[i] = (short)(std::max(-1.f, std::min(1.f, s)) * 28000);
  }
  w.data = d;
  return w;
}

Wave AudioEngine::MakeActionLoop(float sec) {
  const int sr = 22050;
  int n = static_cast<int>(sec * sr);
  Wave w{};
  w.frameCount = n; w.sampleRate = sr; w.sampleSize = 16; w.channels = 1;
  auto* d = (short*)MemAlloc(n * sizeof(short));
  for (int i = 0; i < n; ++i) {
    float t = (float)i / sr;
    float kick = (fmodf(t, 0.3f) < 0.04f) ? sinf(2*3.14159f*80*t) * expf(-fmodf(t,0.3f)*25.f) : 0.f;
    float lead = sinf(2*3.14159f*(200.f + fmodf(t*8.f, 1.f)*100.f)*t) * 0.12f;
    float s = (kick + lead) * 0.55f;
    d[i] = (short)(std::max(-1.f, std::min(1.f, s)) * 30000);
  }
  w.data = d;
  return w;
}

Wave AudioEngine::MakeClubLoop(float sec) {
  const int sr = 22050;
  int n = static_cast<int>(sec * sr);
  Wave w{};
  w.frameCount = n; w.sampleRate = sr; w.sampleSize = 16; w.channels = 1;
  auto* d = (short*)MemAlloc(n * sizeof(short));
  for (int i = 0; i < n; ++i) {
    float t = (float)i / sr;
    float kick = (fmodf(t, 0.5f) < 0.06f) ? sinf(2*3.14159f*50*t) * expf(-fmodf(t,0.5f)*15.f) : 0.f;
    float bass = sinf(2*3.14159f*45*t) * 0.35f;
    float s = (kick * 0.9f + bass) * 0.5f;
    d[i] = (short)(std::max(-1.f, std::min(1.f, s)) * 30000);
  }
  w.data = d;
  return w;
}

Wave AudioEngine::MakeTensionLoop(float sec) {
  const int sr = 22050;
  int n = static_cast<int>(sec * sr);
  Wave w{};
  w.frameCount = n; w.sampleRate = sr; w.sampleSize = 16; w.channels = 1;
  auto* d = (short*)MemAlloc(n * sizeof(short));
  for (int i = 0; i < n; ++i) {
    float t = (float)i / sr;
    float heart = (fmodf(t, 0.85f) < 0.08f) ? sinf(2*3.14159f*40*t) * expf(-fmodf(t,0.85f)*12.f) : 0.f;
    float drone = sinf(2*3.14159f*55*t) * 0.12f + sinf(2*3.14159f*56.5f*t) * 0.1f;
    float s = (heart * 0.7f + drone) * 0.55f;
    d[i] = (short)(std::max(-1.f, std::min(1.f, s)) * 28000);
  }
  w.data = d;
  return w;
}

Wave AudioEngine::MakeRainLoop(float sec) {
  const int sr = 22050;
  int n = static_cast<int>(sec * sr);
  Wave w{};
  w.frameCount = n; w.sampleRate = sr; w.sampleSize = 16; w.channels = 1;
  auto* d = (short*)MemAlloc(n * sizeof(short));
  for (int i = 0; i < n; ++i) {
    float s = ((rand() % 1000) / 1000.f - 0.5f) * 0.25f;
    s += sinf((float)i * 0.01f) * 0.02f;
    d[i] = (short)(std::max(-1.f, std::min(1.f, s)) * 22000);
  }
  w.data = d;
  return w;
}

Wave AudioEngine::MakeVictorySting(float sec) {
  const int sr = 22050;
  int n = static_cast<int>(sec * sr);
  Wave w{};
  w.frameCount = n; w.sampleRate = sr; w.sampleSize = 16; w.channels = 1;
  auto* d = (short*)MemAlloc(n * sizeof(short));
  float notes[] = {261.63f, 329.63f, 392.f, 523.25f};
  for (int i = 0; i < n; ++i) {
    float t = (float)i / sr;
    int ni = std::min(3, (int)(t / (sec * 0.25f)));
    float s = sinf(2*3.14159f*notes[ni]*t) * 0.3f * (1.f - t / sec);
    d[i] = (short)(std::max(-1.f, std::min(1.f, s)) * 30000);
  }
  w.data = d;
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
  music_[0] = LoadFromWaveOwned(MakePhonkLoop(4.f));
  music_[1] = LoadFromWaveOwned(MakeLofiLoop(5.f));
  music_[2] = LoadFromWaveOwned(MakeActionLoop(3.5f));
  music_[3] = LoadFromWaveOwned(MakeClubLoop(4.f));
  music_[4] = LoadFromWaveOwned(MakeTensionLoop(4.f));
  music_[5] = LoadFromWaveOwned(MakeRainLoop(3.f));
  music_[6] = LoadFromWaveOwned(MakeVictorySting(2.f));
  for (int i = 0; i < kTracks; ++i) musicValid_[i] = music_[i].frameCount > 0;

  sfx_[(int)SfxId::Click] = LoadFromWaveOwned(MakeTone(800.f, 0.05f, 0.4f, false));
  sfx_[(int)SfxId::Hover] = LoadFromWaveOwned(MakeTone(600.f, 0.03f, 0.2f, false));
  sfx_[(int)SfxId::Coin] = LoadFromWaveOwned(MakeTone(1200.f, 0.12f, 0.35f, false));
  sfx_[(int)SfxId::Notify] = LoadFromWaveOwned(MakeTone(440.f, 0.15f, 0.3f, false));
  sfx_[(int)SfxId::TireScreech] = LoadFromWaveOwned(MakeTone(900.f, 0.25f, 0.3f, true));
  sfx_[(int)SfxId::BlowOff] = LoadFromWaveOwned(MakeTone(200.f, 0.2f, 0.4f, true));
  sfx_[(int)SfxId::Gunshot] = LoadFromWaveOwned(MakeTone(150.f, 0.15f, 0.5f, true));
  sfx_[(int)SfxId::Footstep] = LoadFromWaveOwned(MakeTone(120.f, 0.08f, 0.25f, true));
  sfx_[(int)SfxId::Ignition] = LoadFromWaveOwned(MakeTone(90.f, 0.35f, 0.4f, true));
  sfx_[(int)SfxId::Siren] = LoadFromWaveOwned(MakeTone(700.f, 0.4f, 0.3f, false));
  for (int i = 0; i < (int)SfxId::Count; ++i) sfxValid_[i] = sfx_[i].frameCount > 0;

  engine_ = LoadFromWaveOwned(MakeTone(80.f, 0.5f, 0.3f, true));
  engineValid_ = engine_.frameCount > 0;
  ready_ = true;
  SetMood(MusicMood::Menu);
}

void AudioEngine::Shutdown() {
  if (!ready_) return;
  for (int i = 0; i < kTracks; ++i) if (musicValid_[i]) UnloadSound(music_[i]);
  for (int i = 0; i < (int)SfxId::Count; ++i) if (sfxValid_[i]) UnloadSound(sfx_[i]);
  if (engineValid_) UnloadSound(engine_);
  CloseAudioDevice();
  ready_ = false;
}

void AudioEngine::SetMood(MusicMood mood) {
  if (mood == targetMood_) return;
  targetMood_ = mood;
  cross_ = 0.f;
}

void AudioEngine::SetMasterVolume(float v) { master_ = std::max(0.f, std::min(1.f, v)); }
void AudioEngine::SetMusicVolume(float v) { musicVol_ = std::max(0.f, std::min(1.f, v)); }
void AudioEngine::SetSfxVolume(float v) { sfxVol_ = std::max(0.f, std::min(1.f, v)); }

void AudioEngine::Update(float dt) {
  if (!ready_) return;
  if (cross_ < 1.f) {
    cross_ = std::min(1.f, cross_ + dt * 1.5f);
    if (cross_ >= 1.f) mood_ = targetMood_;
  }
  int idx = -1;
  MusicMood m = cross_ >= 1.f ? mood_ : targetMood_;
  switch (m) {
    case MusicMood::Menu: idx = 0; break;
    case MusicMood::City: idx = 1; break;
    case MusicMood::Chase: idx = 2; break;
    case MusicMood::Club: idx = 3; break;
    case MusicMood::Tension: idx = 4; break;
    case MusicMood::RainAlley: idx = 5; break;
    case MusicMood::Victory: idx = 6; break;
    default: idx = -1; break;
  }
  if (idx != musicPlaying_) {
    if (musicPlaying_ >= 0 && musicValid_[musicPlaying_]) StopSound(music_[musicPlaying_]);
    musicPlaying_ = idx;
    if (idx >= 0 && musicValid_[idx]) {
      PlaySound(music_[idx]);
      SetSoundVolume(music_[idx], musicVol_ * master_ * 0.55f);
    }
  }
  if (musicPlaying_ >= 0 && musicValid_[musicPlaying_] && !IsSoundPlaying(music_[musicPlaying_])) {
    PlaySound(music_[musicPlaying_]);
    SetSoundVolume(music_[musicPlaying_], musicVol_ * master_ * 0.55f);
  }
}

void AudioEngine::PlaySfx(SfxId id, float pitch, float vol) {
  int i = (int)id;
  if (i < 0 || i >= (int)SfxId::Count || !sfxValid_[i]) return;
  SetSoundPitch(sfx_[i], pitch);
  SetSoundVolume(sfx_[i], vol * sfxVol_ * master_);
  PlaySound(sfx_[i]);
}

void AudioEngine::SetEngine(float rpm01, bool drifting, bool nitro) {
  if (!engineValid_) return;
  rpm01 = std::max(0.f, std::min(1.f, rpm01));
  SetSoundPitch(engine_, 0.7f + rpm01 * 0.8f);
  SetSoundVolume(engine_, rpm01 * 0.4f * sfxVol_ * master_);
  if (rpm01 > 0.05f && !IsSoundPlaying(engine_)) PlaySound(engine_);
  if (rpm01 <= 0.05f && IsSoundPlaying(engine_)) StopSound(engine_);
  if (drifting && rpm01 > 0.2f) PlaySfx(SfxId::TireScreech, 0.9f + rpm01 * 0.2f, 0.35f);
  if (nitro) PlaySfx(SfxId::BlowOff, 1.f, 0.4f);
}

void AudioEngine::SetRadio(RadioStation st) { radio_ = st; }
void AudioEngine::NextRadio() {
  radio_ = static_cast<RadioStation>((static_cast<int>(radio_) + 1) % static_cast<int>(RadioStation::Count));
  switch (radio_) {
    case RadioStation::StreetFM: SetMood(MusicMood::Menu); break;
    case RadioStation::RetroWave: SetMood(MusicMood::City); break;
    case RadioStation::CrimeChannel: SetMood(MusicMood::Tension); break;
    case RadioStation::NightJazz: SetMood(MusicMood::Club); break;
    default: break;
  }
}

const char* AudioEngine::RadioName() const {
  const char* n[] = {"Ulichnyy FM", "Retro-Volna", "Kriminal'nyy Kanal", "Nochnoy Jazz"};
  return n[static_cast<int>(radio_) % 4];
}
