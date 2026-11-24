#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>

struct TrackedTexture {
  Texture2D tex{};
  int refs = 0;
  bool valid = false;
};

struct TrackedModel {
  Model model{};
  int refs = 0;
  bool valid = false;
  bool fromFile = false;
};

struct TrackedShader {
  Shader sh{};
  int refs = 0;
  bool valid = false;
};

class ResourceManager {
public:
  static ResourceManager& Get();

  Texture2D* AcquireTexture(const std::string& path, Color fallbackColor = {80, 80, 100, 255});
  void ReleaseTexture(const std::string& path);

  Model* AcquireModel(const std::string& path);
  void ReleaseModel(const std::string& path);

  Shader* AcquireShader(const std::string& vs, const std::string& fs);
  void ReleaseShader(const std::string& key);

  void BeginPreload(const std::vector<std::string>& texturePaths,
                    const std::vector<std::string>& modelPaths);
  bool PreloadStep(); // one asset per call; returns true when done
  float PreloadProgress() const;
  bool PreloadDone() const { return preloadDone_; }

  void UnloadLocation(const std::string& tag);
  void Tag(const std::string& key, const std::string& locationTag);

  size_t TextureCount() const { return textures_.size(); }
  size_t ModelCount() const { return models_.size(); }
  std::string MemInfo() const;

  void Shutdown();

private:
  ResourceManager() = default;
  Texture2D MakeFallback(Color c);
  std::unordered_map<std::string, TrackedTexture> textures_;
  std::unordered_map<std::string, TrackedModel> models_;
  std::unordered_map<std::string, TrackedShader> shaders_;
  std::unordered_map<std::string, std::string> tags_;
  std::vector<std::string> preloadTex_;
  std::vector<std::string> preloadModel_;
  size_t preloadIndex_ = 0;
  bool preloadDone_ = true;
  size_t preloadTotal_ = 0;
};

// Simple VFX helpers used with postprocess uniforms
struct VfxState {
  float bloom = 0.35f;
  float vignette = 0.25f;
  float motionBlur = 0.f;
  float nitroAberration = 0.f;
  float camShake = 0.f;
  float lowHpPulse = 0.f;
  float wetRoad = 0.f;
  float rain = 0.f;
  void Update(float dt) {
    if (dt > 0.1f) dt = 0.1f;
    motionBlur *= (1.f - dt * 3.f);
    nitroAberration *= (1.f - dt * 2.f);
    camShake *= (1.f - dt * 5.f);
    if (motionBlur < 0.01f) motionBlur = 0;
    if (nitroAberration < 0.01f) nitroAberration = 0;
    if (camShake < 0.01f) camShake = 0;
  }
  void OnNitro() { motionBlur = 1.f; nitroAberration = 1.f; }
  void OnImpact(float strength) { camShake = strength; }
  void OnLowHp(float t) { lowHpPulse = t; }
};
