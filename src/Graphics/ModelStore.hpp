#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>
#include <vector>

enum class CarBodyKind { Sedan, SUV, Hyper, Muscle, Classic };

class ModelStore {
public:
  static ModelStore& Get();
  void Init(const std::string& root = "assets/models");
  void Shutdown();

  Model* GetCharacter(const std::string& id);
  Model* GetCar(const std::string& id, CarBodyKind kind = CarBodyKind::Sedan);
  Model* GetBuilding(int index);

  // procedural mesh models kept for fallback
  Model& ProceduralChar();
  Model& ProceduralCar(CarBodyKind kind);
  Model& ProceduralBuilding();

private:
  ModelStore() = default;
  Model LoadOrEmpty(const std::string& path);
  Model MakeCharMesh();
  Model MakeCarMesh(CarBodyKind kind);
  Model MakeBuildingMesh();

  std::string root_;
  std::unordered_map<std::string, Model> cache_;
  Model procChar_{};
  Model procCar_[5]{};
  Model procBld_{};
  bool ready_ = false;
};
