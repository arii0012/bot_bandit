#include "Core/ResourceManager.hpp"
#include <sstream>
#include <cstdio>

ResourceManager& ResourceManager::Get() {
  static ResourceManager inst;
  return inst;
}

Texture2D ResourceManager::MakeFallback(Color c) {
  Image img = GenImageColor(4, 4, c);
  Texture2D t = LoadTextureFromImage(img);
  UnloadImage(img);
  return t;
}

Texture2D* ResourceManager::AcquireTexture(const std::string& path, Color fallbackColor) {
  auto it = textures_.find(path);
  if (it != textures_.end()) {
    it->second.refs++;
    return &it->second.tex;
  }
  TrackedTexture tr;
  if (!path.empty() && FileExists(path.c_str())) {
    tr.tex = LoadTexture(path.c_str());
    tr.valid = tr.tex.id != 0;
  }
  if (!tr.valid) {
    tr.tex = MakeFallback(fallbackColor);
    tr.valid = true;
  }
  tr.refs = 1;
  textures_[path] = tr;
  return &textures_[path].tex;
}

void ResourceManager::ReleaseTexture(const std::string& path) {
  auto it = textures_.find(path);
  if (it == textures_.end()) return;
  it->second.refs--;
  if (it->second.refs <= 0) {
    if (it->second.tex.id) UnloadTexture(it->second.tex);
    textures_.erase(it);
  }
}

Model* ResourceManager::AcquireModel(const std::string& path) {
  auto it = models_.find(path);
  if (it != models_.end()) {
    it->second.refs++;
    return &it->second.model;
  }
  TrackedModel tr;
  if (!path.empty() && FileExists(path.c_str())) {
    tr.model = LoadModel(path.c_str());
    tr.valid = tr.model.meshCount > 0;
    tr.fromFile = tr.valid;
  }
  if (!tr.valid) {
    Mesh cube = GenMeshCube(1.f, 1.f, 1.f);
    tr.model = LoadModelFromMesh(cube);
    tr.valid = true;
    tr.fromFile = false;
  }
  tr.refs = 1;
  models_[path] = tr;
  return &models_[path].model;
}

void ResourceManager::ReleaseModel(const std::string& path) {
  auto it = models_.find(path);
  if (it == models_.end()) return;
  it->second.refs--;
  if (it->second.refs <= 0) {
    if (it->second.valid) UnloadModel(it->second.model);
    models_.erase(it);
  }
}

Shader* ResourceManager::AcquireShader(const std::string& vs, const std::string& fs) {
  std::string key = vs + "|" + fs;
  auto it = shaders_.find(key);
  if (it != shaders_.end()) {
    it->second.refs++;
    return &it->second.sh;
  }
  TrackedShader tr;
  if (FileExists(vs.c_str()) && FileExists(fs.c_str())) {
    tr.sh = LoadShader(vs.c_str(), fs.c_str());
    tr.valid = tr.sh.id != 0;
  }
  if (!tr.valid) {
    tr.sh = LoadShader(nullptr, nullptr);
    tr.valid = true;
  }
  tr.refs = 1;
  shaders_[key] = tr;
  return &shaders_[key].sh;
}

void ResourceManager::ReleaseShader(const std::string& key) {
  auto it = shaders_.find(key);
  if (it == shaders_.end()) return;
  it->second.refs--;
  if (it->second.refs <= 0) {
    if (it->second.sh.id) UnloadShader(it->second.sh);
    shaders_.erase(it);
  }
}

void ResourceManager::BeginPreload(const std::vector<std::string>& texturePaths,
                                   const std::vector<std::string>& modelPaths) {
  preloadTex_ = texturePaths;
  preloadModel_ = modelPaths;
  preloadIndex_ = 0;
  preloadTotal_ = preloadTex_.size() + preloadModel_.size();
  preloadDone_ = preloadTotal_ == 0;
}

bool ResourceManager::PreloadStep() {
  if (preloadDone_) return true;
  if (preloadIndex_ < preloadTex_.size()) {
    AcquireTexture(preloadTex_[preloadIndex_]);
    preloadIndex_++;
  } else {
    size_t mi = preloadIndex_ - preloadTex_.size();
    if (mi < preloadModel_.size()) {
      AcquireModel(preloadModel_[mi]);
      preloadIndex_++;
    }
  }
  if (preloadIndex_ >= preloadTotal_) preloadDone_ = true;
  return preloadDone_;
}

float ResourceManager::PreloadProgress() const {
  if (preloadTotal_ == 0) return 1.f;
  return (float)preloadIndex_ / (float)preloadTotal_;
}

void ResourceManager::Tag(const std::string& key, const std::string& locationTag) {
  tags_[key] = locationTag;
}

void ResourceManager::UnloadLocation(const std::string& tag) {
  std::vector<std::string> kill;
  for (auto& [k, t] : tags_) {
    if (t == tag) kill.push_back(k);
  }
  for (auto& k : kill) {
    ReleaseTexture(k);
    ReleaseModel(k);
    tags_.erase(k);
  }
}

std::string ResourceManager::MemInfo() const {
  char buf[256];
  snprintf(buf, sizeof(buf), "textures=%zu models=%zu shaders=%zu",
           textures_.size(), models_.size(), shaders_.size());
  return buf;
}

void ResourceManager::Shutdown() {
  for (auto& [_, t] : textures_) {
    if (t.tex.id) UnloadTexture(t.tex);
  }
  textures_.clear();
  for (auto& [_, m] : models_) {
    if (m.valid) UnloadModel(m.model);
  }
  models_.clear();
  for (auto& [_, s] : shaders_) {
    if (s.sh.id) UnloadShader(s.sh);
  }
  shaders_.clear();
  tags_.clear();
}
