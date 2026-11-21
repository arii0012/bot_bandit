#include "Business.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "World/City.hpp"
#include <cmath>

// store business level in materials map under key "biz_<city>"
static int& BizLvl(Character& p, const std::string& city) {
  return p.Inv().materials["biz_" + city];
}

bool BusinessSystem::BuyOrUpgrade(Character& p, const World& w, std::string& msg) {
  auto* c = w.Current();
  if (!c) { msg = "Нет города"; return false; }
  int lvl = BizLvl(p, c->id);
  int cost = (lvl == 0) ? 15000 + (int)(c->priceMult * 10000) : (25000 * (lvl + 1));
  if (p.Money() < cost) { msg = "Нужно " + std::to_string(cost); return false; }
  p.AddMoney(-cost);
  BizLvl(p, c->id) = lvl + 1;
  msg = (lvl == 0 ? "Бизнес открыт в " : "Апгрейд бизнеса в ") + c->name + " (ур." + std::to_string(lvl + 1) + ")";
  return true;
}

int BusinessSystem::Collect(Character& p, const World& w, std::string& msg) {
  auto* c = w.Current();
  if (!c) { msg = "Нет города"; return 0; }
  int lvl = BizLvl(p, c->id);
  if (lvl <= 0) { msg = "Нет бизнеса здесь"; return 0; }
  int income = lvl * 3500 + p.Level() * 200;
  p.AddMoney(income);
  msg = "Собрано с бизнеса: " + std::to_string(income);
  return income;
}

int BusinessSystem::PassiveIncomePerHour(const Character& p) {
  int total = 0;
  for (auto& [k, v] : p.Inv().materials) {
    if (k.rfind("biz_", 0) == 0 && v > 0) total += v * 2000;
  }
  return total;
}
