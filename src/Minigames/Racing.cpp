#include "Racing.hpp"
#include "Character/Character.hpp"
#include "Systems/Weather.hpp"
#include <cstdlib>
#include <string>

std::string StreetRace::Race(Character& p, WeatherSystem* weather, bool pinkSlip) {
  if (p.ActiveCar().empty()) return "Нужна машина";
  if (p.Energy() < 20) return "Нужно 20 энергии";
  p.SetEnergy(p.Energy() - 18);

  float power = 1.0f + p.engineLvl_ * 0.12f + p.turboLvl_ * 0.15f;
  power += p.Skill("driving") * 0.06f;
  if (p.ActiveCar().find("Lambo") != std::string::npos || p.ActiveCar().find("Tesla") != std::string::npos
      || p.ActiveCar().find("Porsche") != std::string::npos) power += 0.35f;
  if (p.ActiveCar().find("BMW") != std::string::npos || p.ActiveCar().find("Mercedes") != std::string::npos)
    power += 0.2f;

  float traction = weather ? weather->TractionMult() : 1.f;
  power *= traction;

  float enemy = 0.9f + (rand() % 100) / 100.f * 0.8f + p.Level() * 0.02f;
  bool win = power >= enemy;

  if (win) {
    p.AddExp(45);
    if (pinkSlip) {
      p.Inv().cars.push_back("Трофей соперника");
      return "PINK SLIP! Забрал тачку соперника";
    }
    int prize = 25000 + p.Level() * 3000 + p.turboLvl_ * 5000;
    p.AddMoney(prize);
    return "Финиш первым! +" + std::to_string(prize);
  }
  p.AddHeat(8);
  if (pinkSlip && !p.Inv().cars.empty()) {
    // risk losing active display only message
    return "Проигрыш pink-slip гонки. Унижение.";
  }
  return "Проигрыш. Соперник ушёл в закат.";
}
