#pragma once
#include "City.hpp"
#include "raylib.h"
#include <string>
#include <vector>

struct Building3D {
  Vector3 pos;
  float w, h, d;
  Color wall;
  bool owned = false;
  int price = 350000;
};

struct TrafficCar {
  Vector3 pos;
  float speed;
  float yaw;
  Color body;
};

class World {
public:
  World();
  void Update(float dt);
  void Draw(int sw, int sh);
  void Draw3DScene(Camera3D cam);

  std::string CurrentCityId() const { return cityId_; }
  std::string CurrentCityName() const;
  const City* Current() const;
  void SetCity(const std::string& id);

  float TimeOfDay() const { return timeOfDay_; }
  bool Raining() const { return raining_; }
  void SetRaining(bool v) { raining_ = v; }

  // house interaction
  int NearHouseIndex(Vector3 playerPos, float radius = 3.f) const;
  bool TryBuyHouse(int idx, class Character& p, std::string& msg);
  bool EnterHouse(int idx);
  void SyncOwnedHouses(const class Character& p);

  const std::vector<Building3D>& Buildings() const { return buildings3_; }

private:
  void EnsureCity();
  void DrawSky3D();
  void DrawGround();
  void DrawBuildings3D();
  void DrawTraffic3D();
  void DrawProps();

  std::string cityId_ = "lipeck";
  float timeOfDay_ = 20.f;
  bool raining_ = false;
  std::vector<Building3D> buildings3_;
  std::vector<TrafficCar> traffic_;
  bool ready_ = false;
  int interiorHouse_ = -1;
};
