#include "Damage.hpp"
#include "Character/Character.hpp"
#include <cmath>
#include <cstdlib>

static BodyDamage gBody;
static VehicleDamage gCar;

BodyDamage& DamageSystem::Body(Character& p) {
  if (p.Inv().materials.count("dmg_hp"))
    gBody.hp = p.Inv().materials["dmg_hp"] / 10.f;
  if (gBody.hp <= 0.f) gBody.hp = 100.f;
  return gBody;
}

VehicleDamage& DamageSystem::Car(Character& p) {
  if (p.Inv().materials.count("cdmg_body")) {
    gCar.body = p.Inv().materials["cdmg_body"] / 10.f;
    gCar.engine = p.Inv().materials["cdmg_eng"] / 10.f;
    gCar.tires = p.Inv().materials["cdmg_tire"] / 10.f;
  }
  return gCar;
}

static void SaveBody(Character& p, const BodyDamage& b) {
  p.Inv().materials["dmg_hp"] = (int)(b.hp * 10);
}

static void SaveCar(Character& p, const VehicleDamage& c) {
  p.Inv().materials["cdmg_body"] = (int)(c.body * 10);
  p.Inv().materials["cdmg_eng"] = (int)(c.engine * 10);
  p.Inv().materials["cdmg_tire"] = (int)(c.tires * 10);
}

void DamageSystem::HitPlayer(Character& p, float amount, const std::string& reason, std::string& msg) {
  auto& b = Body(p);
  b.hp = std::max(0.f, b.hp - amount);
  if (amount > 15.f) b.bleed = std::min(5.f, b.bleed + amount * 0.05f);
  if (amount > 25.f) b.limp = std::min(0.6f, b.limp + 0.15f);
  p.SetEnergy(std::max(0, p.Energy() - (int)(amount * 0.4f)));
  if (b.hp <= 0.f) {
    b.knocked = true;
    b.hp = 5.f;
    p.AddMoney(-(int64_t)(5000 + p.Level() * 500));
    msg = "Нокаут! " + reason;
  } else {
    msg = "Урон " + std::to_string((int)amount) + " (" + reason + ") HP " + std::to_string((int)b.hp);
  }
  SaveBody(p, b);
}

void DamageSystem::HealPlayer(Character& p, float amount, std::string& msg) {
  auto& b = Body(p);
  b.hp = std::min(100.f, b.hp + amount);
  b.bleed = std::max(0.f, b.bleed - amount * 0.1f);
  b.limp = std::max(0.f, b.limp - 0.2f);
  b.knocked = false;
  auto& items = p.Inv().items;
  for (auto it = items.begin(); it != items.end(); ++it) {
    if (*it == "Аптечка" || *it == "medkit") { items.erase(it); break; }
  }
  SaveBody(p, b);
  msg = "Лечение HP " + std::to_string((int)b.hp);
}

void DamageSystem::Tick(Character& p, float dt, std::string& outEvent) {
  outEvent.clear();
  auto& b = Body(p);
  if (b.bleed > 0.01f) {
    b.hp = std::max(0.f, b.hp - b.bleed * dt * 2.f);
    b.bleed = std::max(0.f, b.bleed - dt * 0.15f);
    if (b.hp <= 0.f) {
      b.knocked = true;
      b.hp = 5.f;
      outEvent = "Кровотечение: критично";
    }
  }
  SaveBody(p, b);
}

void DamageSystem::CrashCar(Character& p, float severity, std::string& msg) {
  if (p.ActiveCar().empty()) { msg = "Нет машины"; return; }
  auto& c = Car(p);
  severity = std::clamp(severity, 0.05f, 1.f);
  c.body = std::max(0.f, c.body - severity * 40.f);
  c.engine = std::max(0.f, c.engine - severity * 25.f);
  c.tires = std::max(0.f, c.tires - severity * 30.f);
  c.visual = std::min(1.f, c.visual + severity * 0.4f);
  if (c.body < 15.f || c.engine < 10.f) {
    c.totaled = true;
    msg = "Авто total. Нужен ремонт.";
  } else {
    msg = "Удар! Кузов " + std::to_string((int)c.body) + "% мотор " + std::to_string((int)c.engine) + "%";
  }
  if (severity > 0.5f) {
    std::string m2;
    HitPlayer(p, severity * 30.f, "авария", m2);
    msg += " | " + m2;
  }
  SaveCar(p, c);
}

void DamageSystem::RepairCar(Character& p, std::string& msg) {
  auto& c = Car(p);
  int cost = (int)((100.f - c.body) * 800 + (100.f - c.engine) * 1200 + (100.f - c.tires) * 400);
  if (cost < 5000) cost = 5000;
  if (p.Money() < cost) { msg = "Ремонт авто: " + std::to_string(cost); return; }
  p.AddMoney(-cost);
  c = VehicleDamage{};
  SaveCar(p, c);
  msg = "Авто восстановлено (-" + std::to_string(cost) + ")";
}

float DamageSystem::PowerMult(const Character& p) {
  VehicleDamage vd = Car(const_cast<Character&>(p));
  if (vd.totaled) return 0.15f;
  return std::max(0.2f, (vd.engine / 100.f) * 0.5f + (vd.tires / 100.f) * 0.3f + 0.2f);
}
