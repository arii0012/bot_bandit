#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>

class AssetManager {
public:
  void LoadAll(const std::string& root);
  Texture2D Get(const std::string& name) const;
  bool Has(const std::string& name) const;
  Texture2D Placeholder(int w, int h, Color c) const;

private:
  std::unordered_map<std::string, Texture2D> textures_;
  std::string root_;
  void TryLoad(const std::string& name, const std::string& path);
};
