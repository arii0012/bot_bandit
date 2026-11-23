#pragma once
#include "Character/Character.hpp"
#include "UITheme.hpp"
#include "CarDatabase150.hpp"
#include <string>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <vector>

// Aggregates 20 gameplay mechanics as callable actions + light state
class Mechanics20 {
public:
  std::string Casino(Character& p, int game) {
    // 0 roulette 1 blackjack 2 slots 3 vip
    int bet = game == 3 ? 50000 : 5000;
    if (p.Money() < bet) return "Need $" + std::to_string(bet);
    p.AddMoney(-bet);
    int roll = GetRandomValue(0, 100);
    int win = 0;
    if (game == 0) win = (roll > 52) ? bet * 2 : 0;
    else if (game == 1) win = (roll > 48) ? (int)(bet * 1.8f) : 0;
    else if (game == 2) win = (roll > 70) ? bet * 5 : (roll > 55 ? bet : 0);
    else win = (roll > 60) ? bet * 3 : 0;
    p.AddMoney(win);
    return win ? ("Casino +" + std::to_string(win)) : "Casino lost";
  }

  std::string TaxiAutopilot(Character& p, const std::string& city) {
    int cost = 3000 + GetRandomValue(0, 4000);
    if (p.Money() < cost) return "Taxi: need money";
    p.AddMoney(-cost);
    p.SetCity(city);
    p.SetEnergy(std::max(0, p.Energy() - 3));
    return "Autopilot -> " + city;
  }

  std::string CryptoTrade(Character& p, bool buy) {
    if (buy) {
      int spend = std::min((int64_t)50000, p.Money());
      if (spend < 1000) return "No funds";
      p.AddMoney(-spend);
      crypto_ += spend / 100.f * (1.f + GetRandomValue(-10, 20) / 100.f);
      return "Bought crypto";
    }
    int cash = (int)(crypto_ * 100.f);
    crypto_ = 0;
    p.AddMoney(cash);
    return "Sold crypto +$" + std::to_string(cash);
  }

  std::string MiningTick(Character& p, float dt) {
    if (!mining_) return {};
    mineTemp_ += dt * 2.f;
    if (mineTemp_ > 90.f) { mining_ = false; return "Mining overheat OFF"; }
    crypto_ += dt * 0.15f * mineCards_;
    return {};
  }

  std::string MiningToggle(Character& p) {
    if (!mining_) {
      if (p.Money() < 20000) return "Need GPUs $20k";
      p.AddMoney(-20000);
      mineCards_++;
      mining_ = true;
      mineTemp_ = 40.f;
      return "Mining farm ON";
    }
    mining_ = false;
    return "Mining OFF";
  }

  std::string Smuggle(Character& p) {
    if (p.Energy() < 20) return "Too tired";
    p.SetEnergy(p.Energy() - 20);
    int risk = 30 + p.Heat();
    if (GetRandomValue(0, 100) < risk) {
      p.AddHeat(15);
      p.SetWanted(std::min(5, p.Wanted() + 1));
      return "Busted at checkpoint";
    }
    int gain = 40000 + GetRandomValue(0, 60000);
    p.AddMoney(gain);
    return "Smuggle +$" + std::to_string(gain);
  }

  std::string DressAuthority(Character& p) {
    int style = (int)p.Inv().clothes.size() * 5 + p.Level();
    authority_ = style;
    return "Authority " + std::to_string(authority_);
  }

  std::string HireGuard(Character& p) {
    if (p.Money() < 100000) return "Guard: $100k";
    p.AddMoney(-100000);
    guards_++;
    return "Bodyguard hired x" + std::to_string(guards_);
  }

  std::string PetDog(Character& p) {
    if (p.Money() < 25000) return "Dog $25k";
    p.AddMoney(-25000);
    hasDog_ = true;
    return "Watchdog at home";
  }

  std::string SafeCrack(Character& p) {
    if (p.Energy() < 10) return "No energy";
    p.SetEnergy(p.Energy() - 10);
    if (GetRandomValue(0, 100) < 40 + p.Skill("hacking")) {
      int g = 15000 + GetRandomValue(0, 35000);
      p.AddMoney(g);
      return "Safe opened +$" + std::to_string(g);
    }
    p.AddHeat(8);
    return "Alarm! Heat up";
  }

  std::string CarTheft(Character& p) {
    if (p.Energy() < 25) return "Need energy";
    p.SetEnergy(p.Energy() - 25);
    if (GetRandomValue(0, 100) < 35 + p.Skill("stealth")) {
      auto& db = CarDatabase150::All();
      auto& c = db[GetRandomValue(0, (int)db.size() - 1)];
      p.Inv().cars.push_back(std::string(c.brand) + " " + c.model);
      p.AddHeat(12);
      return std::string("Stolen ") + c.brand + " " + c.model;
    }
    p.SetWanted(std::min(5, p.Wanted() + 2));
    return "Theft failed — wanted";
  }

  std::string Bribe(Character& p) {
    int cost = 20000 * std::max(1, p.Wanted());
    if (p.Money() < cost) return "Bribe $" + std::to_string(cost);
    p.AddMoney(-cost);
    p.SetWanted(0);
    p.AddHeat(-20);
    return "Police paid off";
  }

  std::string Gym(Character& p) {
    if (p.Money() < 2000) return "Gym $2k";
    p.AddMoney(-2000);
    p.SetEnergy(std::min(100, p.Energy() + 15));
    p.AddStat("strength", 1);
    hunger_ = std::max(0.f, hunger_ - 10.f);
    return "Gym: strength up";
  }

  void NeedsTick(Character& p, float dt) {
    hunger_ = std::min(100.f, hunger_ + dt * 0.4f);
    if (hunger_ > 80.f) p.SetEnergy(std::max(0, p.Energy() - (int)(dt * 2)));
  }

  std::string Eat(Character& p) {
    if (p.Money() < 500) return "Food $500";
    p.AddMoney(-500);
    hunger_ = std::max(0.f, hunger_ - 40.f);
    p.SetEnergy(std::min(100, p.Energy() + 10));
    return "Ate meal";
  }

  std::string PhotoMode(Character& p) {
    int gain = 2000 + p.Level() * 300;
    p.AddMoney(gain);
    return "Insta post +$" + std::to_string(gain);
  }

  std::string Romance(Character& p) {
    if (p.Money() < 10000) return "Date $10k";
    p.AddMoney(-10000);
    relation_ = std::min(100, relation_ + GetRandomValue(5, 20));
    return "Relation " + std::to_string(relation_) + "%";
  }

  std::string RacePinkSlips(Character& p) {
    if (p.ActiveCar().empty()) return "Need a car";
    if (GetRandomValue(0, 100) < 45 + p.Skill("driving")) {
      p.AddMoney(80000);
      return "Pink slip race WIN +$80k";
    }
    if (!p.Inv().cars.empty()) {
      p.Inv().cars.pop_back();
      p.SetActiveCar(p.Inv().cars.empty() ? "" : p.Inv().cars.back());
    }
    return "Lost the car";
  }

  float Crypto() const { return crypto_; }
  int Authority() const { return authority_; }
  int Guards() const { return guards_; }
  bool Dog() const { return hasDog_; }
  float Hunger() const { return hunger_; }
  int Relation() const { return relation_; }

private:
  float crypto_ = 0.f;
  bool mining_ = false;
  int mineCards_ = 0;
  float mineTemp_ = 40.f;
  int authority_ = 0;
  int guards_ = 0;
  bool hasDog_ = false;
  float hunger_ = 20.f;
  int relation_ = 0;
};
