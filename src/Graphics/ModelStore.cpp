#include "ModelStore.hpp"
#include <filesystem>

void ModelStore::LoadOptional(const std::string& id, const std::string& path) {
  if (!std::filesystem::exists(path)) return;
  Model m = LoadModel(path.c_str());
  if (m.meshCount > 0) models_[id] = m;
}

bool ModelStore::Has(const std::string& id) const {
  return models_.count(id) > 0;
}

Model* ModelStore::Get(const std::string& id) {
  auto it = models_.find(id);
  return it == models_.end() ? nullptr : &it->second;
}

void ModelStore::UnloadAll() {
  for (auto& [_, m] : models_) UnloadModel(m);
  models_.clear();
}
