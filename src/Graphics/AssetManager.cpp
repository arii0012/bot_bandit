#include "AssetManager.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void AssetManager::LoadAll(const std::string& root) {
  root_ = root;
  if (!fs::exists(root)) {
    fs::create_directories(root);
  }

  // load all png recursively under assets
  if (fs::exists(root)) {
    for (auto& p : fs::recursive_directory_iterator(root)) {
      if (!p.is_regular_file()) continue;
      auto ext = p.path().extension().string();
      if (ext != ".png" && ext != ".PNG") continue;
      std::string key = p.path().stem().string();
      TryLoad(key, p.path().string());
    }
  }

  // ensure placeholders for critical keys
  const char* need[] = {
    "char_base", "bg_city", "phone", "cursor",
    "car_vaz", "car_lambo", "car_bmw", "house_room", "house_mansion"
  };
  for (auto* k : need) {
    if (!Has(k)) {
      Image img = GenImageColor(64, 64, Color{40, 40, 50, 255});
      ImageDrawRectangle(&img, 4, 4, 56, 56, Color{80, 80, 100, 255});
      Texture2D t = LoadTextureFromImage(img);
      UnloadImage(img);
      textures_[k] = t;
    }
  }
}

void AssetManager::TryLoad(const std::string& name, const std::string& path) {
  if (textures_.count(name)) return;
  Texture2D t = LoadTexture(path.c_str());
  if (t.id != 0) {
    textures_[name] = t;
  }
}

Texture2D AssetManager::Get(const std::string& name) const {
  auto it = textures_.find(name);
  if (it != textures_.end()) return it->second;
  // return first available or empty
  if (!textures_.empty()) return textures_.begin()->second;
  static Texture2D empty{};
  return empty;
}

bool AssetManager::Has(const std::string& name) const {
  return textures_.count(name) > 0;
}

Texture2D AssetManager::Placeholder(int w, int h, Color c) const {
  Image img = GenImageColor(w, h, c);
  Texture2D t = LoadTextureFromImage(img);
  UnloadImage(img);
  return t;
}
