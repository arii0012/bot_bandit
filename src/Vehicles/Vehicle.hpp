#pragma once
#include <string>

struct VehicleData {
  std::string id;
  std::string name;
  int price = 0;
  float speed = 1.f;
  int tank = 50;
};

class Vehicle {
public:
  explicit Vehicle(const VehicleData& d) : data_(d) {}
  const VehicleData& Data() const { return data_; }
  float Fuel() const { return fuel_; }
  void Refuel(float v) { fuel_ += v; if (fuel_ > data_.tank) fuel_ = (float)data_.tank; }

private:
  VehicleData data_;
  float fuel_ = 50.f;
};
