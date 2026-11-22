#include "CarCustomizer.hpp"
#include "Character/Character.hpp"
#include "Core/Application.hpp"
#include "UI/UISystem.hpp"
#include "Systems/GameSystems.hpp"
#include <map>
#include <cstdlib>

static std::map<std::string, CarVisual> gVis;

CarVisual& CarCustomizer::Visual(Character& p) {
  std::string key = p.ActiveCar().empty() ? "_none" : p.ActiveCar();
  auto& v = gVis[key];
  // sync legacy fields
  v.engineLvl = p.engineLvl_;
  v.turboLvl = p.turboLvl_;
  v.neon = p.neonCar_;
  v.r = p.carR_; v.g = p.carG_; v.b = p.carB_;
  return v;
}

std::string CarCustomizer::Apply(Character& p, const std::string& opt, std::string& msg) {
  if (p.ActiveCar().empty()) { msg = "Нет активной машины"; return msg; }
  auto& v = Visual(p);
  auto pay = [&](int cost) -> bool {
    if (p.Money() < cost) { msg = "Нужно " + std::to_string(cost); return false; }
    p.AddMoney(-cost);
    return true;
  };

  if (opt == "paint_mat") {
    if (!pay(45000)) return msg;
    v.metallic = false; v.chameleon = false;
    v.r = rand()%180+40; v.g = rand()%180+40; v.b = rand()%180+40;
    p.carR_ = v.r; p.carG_ = v.g; p.carB_ = v.b;
    msg = "Матовая покраска";
  } else if (opt == "paint_metal") {
    if (!pay(75000)) return msg;
    v.metallic = true; v.chameleon = false;
    v.r = rand()%100+120; v.g = rand()%100+120; v.b = rand()%100+120;
    p.carR_ = v.r; p.carG_ = v.g; p.carB_ = v.b;
    msg = "Металлик";
  } else if (opt == "paint_chameleon") {
    if (!pay(150000)) return msg;
    v.chameleon = true; v.metallic = true;
    v.r = 180; v.g = 40; v.b = 200;
    p.carR_ = v.r; p.carG_ = v.g; p.carB_ = v.b;
    msg = "Хамелеон";
  } else if (opt == "neon") {
    if (!pay(80000)) return msg;
    v.neon = true; p.neonCar_ = true;
    msg = "Неон под днищем";
  } else if (opt == "spoiler") {
    if (!pay(55000)) return msg;
    v.spoiler = std::min(2, v.spoiler + 1);
    msg = "Спойлер ур." + std::to_string(v.spoiler);
  } else if (opt == "wheels") {
    if (!pay(40000)) return msg;
    v.wheels = (v.wheels + 1) % 4;
    msg = "Диски #" + std::to_string(v.wheels);
  } else if (opt == "vinyl") {
    if (!pay(60000)) return msg;
    v.wrap = (v.wrap + 1) % 5;
    msg = "Винил #" + std::to_string(v.wrap);
  } else if (opt == "suspension") {
    if (!pay(90000)) return msg;
    v.suspension = std::min(2, v.suspension + 1);
    msg = "Подвеска -" + std::to_string(v.suspension);
  } else if (opt == "xenon") {
    if (!pay(35000)) return msg;
    v.xenon = true;
    msg = "Ксенон";
  } else if (opt == "tires_soft") {
    if (!pay(95000)) return msg;
    v.wheels = 0; // reuse as compound soft marker in wrap high bits - store in suspension field hack
    p.Inv().materials["tire_compound"] = 0;
    msg = "Шины Soft (макс grip сухо, слабо в дождь)";
  } else if (opt == "tires_med") {
    if (!pay(70000)) return msg;
    p.Inv().materials["tire_compound"] = 1;
    msg = "Шины Medium";
  } else if (opt == "tires_hard") {
    if (!pay(55000)) return msg;
    p.Inv().materials["tire_compound"] = 2;
    msg = "Шины Hard (стабильны в дождь)";
  } else if (opt == "bodykit") {
    if (!pay(180000)) return msg;
    v.spoiler = 2;
    v.suspension = std::min(2, v.suspension + 1);
    msg = "Обвес + низкая посадка";
  } else if (opt == "plate") {
    if (!pay(15000)) return msg;
    v.plate = "BOT" + std::to_string(100 + rand()%900);
    msg = "Номер: " + v.plate;
  } else {
    msg = "unknown opt";
  }
  return msg;
}
