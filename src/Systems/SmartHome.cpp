#include "SmartHome.hpp"
#include "Character/Character.hpp"
#include <algorithm>

static int Flag(Character& p, const std::string& k) {
  return p.Inv().materials[k];
}
static void SetFlag(Character& p, const std::string& k, int v) {
  p.Inv().materials[k] = v;
}

bool SmartHome::HasMansion(const Character& p) {
  for (auto& h : p.Inv().houses)
    if (h.find("Особняк") != std::string::npos || h.find("mansion") != std::string::npos) return true;
  // also if bought via item tag
  for (auto& it : p.Inv().items)
    if (it == "house_mansion") return true;
  return Flag(const_cast<Character&>(p), "house_mansion") > 0;
}

std::string SmartHome::ToggleLight(Character& p) {
  if (!HasMansion(p)) return "Нужен особняк";
  int v = Flag(p, "sh_light") ? 0 : 1;
  SetFlag(p, "sh_light", v);
  return v ? "Свет включён" : "Свет выключен";
}

std::string SmartHome::ToggleAlarm(Character& p) {
  if (!HasMansion(p)) return "Нужен особняк";
  int v = Flag(p, "sh_alarm") ? 0 : 1;
  SetFlag(p, "sh_alarm", v);
  return v ? "Сигнализация ON" : "Сигнализация OFF";
}

std::string SmartHome::OpenSafe(Character& p) {
  if (!HasMansion(p)) return "Нужен особняк";
  if (Flag(p, "sh_safe_open")) {
    int cash = 5000 + p.Level() * 500;
    p.AddMoney(cash);
    return "Сейф: +" + std::to_string(cash) + " (заначка)";
  }
  SetFlag(p, "sh_safe_open", 1);
  p.AddMoney(25000);
  return "Сейф взломан / открыт: +25000";
}

std::string SmartHome::Party(Character& p) {
  if (!HasMansion(p)) return "Нужен особняк";
  if (p.Money() < 30000) return "Вечеринка стоит 30к";
  p.AddMoney(-30000);
  p.SetEnergy(100);
  p.AddExp(30);
  // rep via skill points light
  p.AddSkillPoints(0);
  return "Вечеринка! Энергия full, +exp. NPC тусуются.";
}

std::string SmartHome::OrderGear(Character& p, const std::string& kind) {
  if (!HasMansion(p)) return "Нужен особняк";
  if (kind == "food") {
    if (p.Money() < 3000) return "Нужно 3000";
    p.AddMoney(-3000);
    p.SetEnergy(std::min(100, p.Energy() + 40));
    return "Еда у двери. Энергия +40";
  }
  if (kind == "weapon") {
    if (p.Money() < 60000) return "Нужно 60000";
    p.AddMoney(-60000);
    p.Inv().weapons.push_back("Пистолет (доставка)");
    return "Оружие у порога";
  }
  return "food|weapon";
}
