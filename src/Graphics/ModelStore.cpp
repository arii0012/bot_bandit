#include "Graphics/ModelStore.hpp"
#include <cmath>
#include <cstdio>

ModelStore& ModelStore::Get() {
  static ModelStore inst;
  return inst;
}

Model ModelStore::LoadOrEmpty(const std::string& path) {
  if (FileExists(path.c_str())) {
    Model m = LoadModel(path.c_str());
    if (m.meshCount > 0) return m;
    UnloadModel(m);
  }
  Model empty{};
  return empty;
}

Model ModelStore::MakeCharMesh() {
  Mesh body = GenMeshCube(0.45f, 0.7f, 0.28f);
  Model m = LoadModelFromMesh(body);
  return m;
}

Model ModelStore::MakeCarMesh(CarBodyKind kind) {
  float w = 1.8f, h = 0.55f, l = 4.2f;
  if (kind == CarBodyKind::SUV) { h = 0.95f; l = 4.4f; w = 2.0f; }
  if (kind == CarBodyKind::Hyper) { h = 0.4f; l = 4.5f; w = 2.1f; }
  if (kind == CarBodyKind::Muscle) { h = 0.5f; l = 4.6f; w = 2.0f; }
  if (kind == CarBodyKind::Classic) { h = 0.6f; l = 4.0f; w = 1.7f; }
  Mesh body = GenMeshCube(w, h, l);
  return LoadModelFromMesh(body);
}

Model ModelStore::MakeBuildingMesh() {
  Mesh box = GenMeshCube(4.f, 8.f, 4.f);
  return LoadModelFromMesh(box);
}

void ModelStore::Init(const std::string& root) {
  if (ready_) return;
  root_ = root;
  procChar_ = MakeCharMesh();
  procCar_[0] = MakeCarMesh(CarBodyKind::Sedan);
  procCar_[1] = MakeCarMesh(CarBodyKind::SUV);
  procCar_[2] = MakeCarMesh(CarBodyKind::Hyper);
  procCar_[3] = MakeCarMesh(CarBodyKind::Muscle);
  procCar_[4] = MakeCarMesh(CarBodyKind::Classic);
  procBld_ = MakeBuildingMesh();
  ready_ = true;
}

void ModelStore::Shutdown() {
  for (auto& [_, m] : cache_) {
    if (m.meshCount > 0) UnloadModel(m);
  }
  cache_.clear();
  if (procChar_.meshCount) UnloadModel(procChar_);
  for (int i = 0; i < 5; ++i) if (procCar_[i].meshCount) UnloadModel(procCar_[i]);
  if (procBld_.meshCount) UnloadModel(procBld_);
  ready_ = false;
}

Model* ModelStore::GetCharacter(const std::string& id) {
  std::string key = "char_" + id;
  auto it = cache_.find(key);
  if (it != cache_.end() && it->second.meshCount > 0) return &it->second;
  std::string path = root_ + "/characters/" + id + ".glb";
  Model m = LoadOrEmpty(path);
  if (m.meshCount == 0) path = root_ + "/char_" + id + ".glb", m = LoadOrEmpty(path);
  if (m.meshCount == 0) return &ProceduralChar();
  cache_[key] = m;
  return &cache_[key];
}

Model* ModelStore::GetCar(const std::string& id, CarBodyKind kind) {
  std::string key = "car_" + id;
  auto it = cache_.find(key);
  if (it != cache_.end() && it->second.meshCount > 0) return &it->second;
  std::string path = root_ + "/cars/" + id + ".glb";
  Model m = LoadOrEmpty(path);
  if (m.meshCount == 0) path = root_ + "/car_" + id + ".glb", m = LoadOrEmpty(path);
  if (m.meshCount == 0) return &ProceduralCar(kind);
  cache_[key] = m;
  return &cache_[key];
}

Model* ModelStore::GetBuilding(int index) {
  std::string key = "bld_" + std::to_string(index);
  auto it = cache_.find(key);
  if (it != cache_.end() && it->second.meshCount > 0) return &it->second;
  char buf[64];
  snprintf(buf, sizeof(buf), "%s/buildings/%02d.glb", root_.c_str(), index % 30);
  Model m = LoadOrEmpty(buf);
  if (m.meshCount == 0) {
    snprintf(buf, sizeof(buf), "%s/bld_%02d.glb", root_.c_str(), index % 30);
    m = LoadOrEmpty(buf);
  }
  if (m.meshCount == 0) return &ProceduralBuilding();
  cache_[key] = m;
  return &cache_[key];
}

Model& ModelStore::ProceduralChar() { return procChar_; }
Model& ModelStore::ProceduralCar(CarBodyKind kind) {
  int i = 0;
  if (kind == CarBodyKind::SUV) i = 1;
  else if (kind == CarBodyKind::Hyper) i = 2;
  else if (kind == CarBodyKind::Muscle) i = 3;
  else if (kind == CarBodyKind::Classic) i = 4;
  return procCar_[i];
}
Model& ModelStore::ProceduralBuilding() { return procBld_; }
