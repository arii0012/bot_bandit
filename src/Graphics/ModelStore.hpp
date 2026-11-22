#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>

class ModelStore {
public:
  void LoadOptional(const std::string& id, const std::string& path);
  bool Has(const std::string& id) const;
  Model* Get(const std::string& id);
  void UnloadAll();

private:
  std::unordered_map<std::string, Model> models_;
};
