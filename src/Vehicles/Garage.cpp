#include "Garage.hpp"
#include "Character/Character.hpp"
#include <cstdlib>

std::vector<VehicleData> Garage::Catalog() {
  return {
    {"vaz", "ВАЗ 2107", 180000, 1.0f, 40},
    {"prior", "Приора", 350000, 1.15f, 45},
    {"bmw", "BMW 5", 2800000, 1.7f, 55},
    {"mercedes", "Mercedes E", 5200000, 1.65f, 60},
    {"tesla", "Tesla Plaid", 14000000, 2.4f, 100},
    {"lambo", "Lamborghini", 25000000, 2.7f, 70},
    {"gclass", "G63 AMG", 18000000, 1.6f, 80},
    {"porsche", "Porsche 911", 12000000, 2.3f, 55},
  };
}

bool Garage::Buy(Character& p, const std::string& carId, std::string& msg) {
  for (auto& c : Catalog()) {
    if (c.id != carId) continue;
    if (p.Money() < c.price) { msg = "Нужно " + std::to_string(c.price); return false; }
    for (auto& owned : p.Inv().cars)
      if (owned == c.name) { msg = "Уже есть"; return false; }
    p.AddMoney(-c.price);
    p.Inv().cars.push_back(c.name);
    p.SetActiveCar(c.name);
    // store tuning meta in items as simple tags
    p.Inv().items.push_back("carmeta:" + c.id);
    msg = "Купил " + c.name;
    return true;
  }
  msg = "Нет такой машины";
  return false;
}

bool Garage::Tune(Character& p, const std::string& what, std::string& msg) {
  if (p.ActiveCar().empty()) { msg = "Нет активной машины"; return false; }
  int cost = 0;
  if (what == "engine") cost = 150000;
  else if (what == "turbo") cost = 220000;
  else if (what == "neon") cost = 80000;
  else if (what == "paint") cost = 45000;
  else { msg = "engine|turbo|neon|paint"; return false; }
  if (p.Money() < cost) { msg = "Нужно " + std::to_string(cost); return false; }
  p.AddMoney(-cost);
  p.Inv().items.push_back(std::string("tune_") + what + ":" + p.ActiveCar());
  msg = "Тюнинг: " + what + " на " + p.ActiveCar();
  return true;
}

OwnedCar* Garage::Active(Character&) { return nullptr; }
