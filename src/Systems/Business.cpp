#include "Business.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "World/City.hpp"
#include <cmath>

static int& BizLvl(Character& p, const std::string& city) {
  return p.Inv().materials["biz_" + city];
}

static const char* BizNameFor(const City& c) {
  if (c.id == "москва") return "Кофейня в центре";
  if (c.id == "сочи") return "Сувенирный";
  if (c.id == "ростов") return "Шаурма-точка";
  if (c.id == "липецк") return "Мороженка";
  if (c.id == "казань") return "Чайхана";
  if (c.id == "питер") return "Арт-бар";
  if (c.id == "владивосток") return "Суши-бар";
  return "Точка";
}

bool BusinessSystem::BuyOrUpgrade(Character& p, const World& w, std::string& msg) {
  auto* c = w.Current();
  if (!c) { msg = "Нет города"; return false; }
  int lvl = BizLvl(p, c->id);
  int cost = (lvl == 0) ? (int)(12000 * c->priceMult) + 8000 : (20000 * (lvl + 1));
  if (p.Money() < cost) { msg = "Нужно " + std::to_string(cost) + " руб."; return false; }
  p.AddMoney(-cost);
  BizLvl(p, c->id) = lvl + 1;
  const char* bn = BizNameFor(*c);
  if (lvl == 0)
    msg = std::string("Открыт бизнес «") + bn + "» в " + c->name;
  else
    msg = std::string("Апгрейд «") + bn + "» → ур." + std::to_string(lvl + 1);
  return true;
}

int BusinessSystem::Collect(Character& p, const World& w, std::string& msg) {
  auto* c = w.Current();
  if (!c) { msg = "Нет города"; return 0; }
  int lvl = BizLvl(p, c->id);
  if (lvl <= 0) { msg = "Нет бизнеса в " + c->name; return 0; }
  int income = lvl * 3200 + p.Level() * 250 + (int)(c->workBonus * 15);
  // negotiation skill bonus
  income += p.Skill("negotiation") * 400;
  p.AddMoney(income);
  msg = std::string("«") + BizNameFor(*c) + "»: +" + std::to_string(income) + " руб.";
  return income;
}

int BusinessSystem::PassiveIncomePerHour(const Character& p) {
  int total = 0;
  for (auto& [k, v] : p.Inv().materials) {
    if (k.rfind("biz_", 0) == 0 && v > 0) total += v * 2200;
  }
  return total;
}
