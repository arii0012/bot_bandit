#pragma once
#include "UIAndSystems.hpp"
#include "CoreEngine.hpp"
#include "WorldAndEntities3D.hpp"
#include "Character/Character.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>

enum class FactionId { Racers, Syndicate, Police, Elite, Hackers };

class FactionSystem {
public:
  int Get(FactionId f) const {
    auto it = rep_.find((int)f);
    return it == rep_.end() ? 0 : it->second;
  }
  void Add(FactionId f, int delta) {
    int v = Get(f) + delta;
    if (v > 100) v = 100;
    if (v < -100) v = -100;
    rep_[(int)f] = v;
  }
  std::string Label(FactionId f) const {
    switch (f) {
      case FactionId::Racers: return "Street Racers";
      case FactionId::Syndicate: return "Syndicate";
      case FactionId::Police: return "Police";
      case FactionId::Elite: return "Business Elite";
      case FactionId::Hackers: return "DarkNet Hackers";
    }
    return "Unknown";
  }
  std::string StatusLine() const {
    return "Rep R" + std::to_string(Get(FactionId::Racers)) +
           " S" + std::to_string(Get(FactionId::Syndicate)) +
           " P" + std::to_string(Get(FactionId::Police)) +
           " E" + std::to_string(Get(FactionId::Elite)) +
           " H" + std::to_string(Get(FactionId::Hackers));
  }
private:
  std::unordered_map<int, int> rep_;
};

class BriberySystem {
public:
  // returns result message; success depends on money, charisma skill, police rep, wanted
  std::string BribePatrol(Character& p, FactionSystem& fac) {
    int stars = p.Wanted();
    if (stars <= 0) return "No heat to bribe";
    if (stars > 3) return "Too hot — bribe on street impossible";
    int base = 15000 * stars;
    int need = base - fac.Get(FactionId::Police) * 100 - p.Skill("charisma") * 500;
    if (need < 5000) need = 5000;
    if (p.Money() < need) return "Bribe needs $" + std::to_string(need);
    p.AddMoney(-need);
    int chance = 55 + fac.Get(FactionId::Police) / 4 + p.Skill("charisma") * 3 - stars * 8;
    if (chance > 90) chance = 90;
    if (GetRandomValue(0, 100) < chance) {
      p.SetWanted(0);
      p.AddHeat(-10);
      fac.Add(FactionId::Police, 2);
      return "Patrol paid off $" + std::to_string(need);
    }
    p.SetWanted(std::min(5, stars + 1));
    fac.Add(FactionId::Police, -5);
    return "Bribe failed — extra star";
  }

  std::string BribeJailChief(Character& p, FactionSystem& fac, WorldAndEntities3D& w) {
    if (!w.Jail().occupied) return "Not in KPZ";
    int need = 200000 - fac.Get(FactionId::Police) * 500;
    if (need < 80000) need = 80000;
    if (p.Money() < need) return "Chief bribe $" + std::to_string(need);
    p.AddMoney(-need);
    int chance = 50 + p.Skill("charisma") * 4 + fac.Get(FactionId::Police) / 5;
    if (GetRandomValue(0, 100) < chance) {
      w.Jail().occupied = false;
      w.Jail().timer = 0;
      fac.Add(FactionId::Police, 3);
      return "Released via chief";
    }
    w.Jail().timer += 60.f;
    fac.Add(FactionId::Police, -8);
    return "Chief refused — longer stay";
  }

  std::string BribeTaxMan(Character& p, FactionSystem& fac) {
    int need = 100000;
    if (p.Money() < need) return "Tax bribe $100k";
    p.AddMoney(-need);
    if (GetRandomValue(0, 100) < 60 + fac.Get(FactionId::Elite) / 5) {
      fac.Add(FactionId::Elite, 5);
      fac.Add(FactionId::Police, -2);
      return "Tax audit closed";
    }
    p.AddHeat(10);
    return "Tax bribe exposed";
  }

  std::string BribeVipGuard(Character& p, FactionSystem& fac) {
    int need = 50000;
    if (p.Money() < need) return "VIP guard $50k";
    if (fac.Get(FactionId::Elite) < -20) return "Elite hates you";
    p.AddMoney(-need);
    fac.Add(FactionId::Elite, 1);
    return "VIP access granted";
  }
};

struct AuctionLot {
  std::string title;
  int price = 0;
  int timer = 15;
  bool container = false;
};

struct FenceNPC {
  std::string name = "Baryga";
  Vector3 pos{12, 0, -18};
  float relocateTimer = 90.f;
  std::vector<std::string> stock = {"twin_turbo", "nos", "fake_passport", "vin_kit"};
};

class DeepSystems {
public:
  FactionSystem factions;
  BriberySystem bribes;

  std::string BuyBusiness(Character& p, const std::string& id, int price) {
    if (p.Money() < price) return "Need $" + std::to_string(price);
    p.AddMoney(-price);
    biz_[id] = {id, 1, 5000, 10};
    factions.Add(FactionId::Elite, 3);
    return "Business owned: " + id;
  }
  std::string HireStaff(Character& p, const std::string& id) {
    if (!biz_.count(id)) return "No business";
    if (p.Money() < 20000) return "Hire $20k";
    p.AddMoney(-20000);
    biz_[id].staff++;
    biz_[id].income += 800;
    return "Staff+ " + id;
  }
  void BusinessTick(Character& p, float dt) {
    day_ += dt;
    if (day_ < 30.f) return;
    day_ = 0;
    for (auto& [id, b] : biz_) {
      int gain = b.income * std::max(1, b.staff);
      int tax = gain / 10;
      if (GetRandomValue(0, 100) < b.risk) {
        p.AddHeat(8);
        factions.Add(FactionId::Syndicate, -2);
        continue;
      }
      p.AddMoney(gain - tax);
      factions.Add(FactionId::Elite, 1);
    }
  }

  std::string Roulette(Character& p, int bet, int number) {
    if (bet <= 0 || p.Money() < bet) return "No bet money";
    if (number < 0 || number > 36) return "Bad number";
    p.AddMoney(-bet);
    int roll = GetRandomValue(0, 36);
    if (roll == number) {
      int win = bet * 35;
      p.AddMoney(win);
      return "Roulette WIN +" + std::to_string(win);
    }
    return "Roulette " + std::to_string(roll) + " lose";
  }
  std::string Blackjack(Character& p, int bet, int intelligence) {
    if (bet <= 0 || p.Money() < bet) return "No bet";
    p.AddMoney(-bet);
    int player = GetRandomValue(16, 21);
    int dealer = GetRandomValue(16, 21);
    if (intelligence > 5) player = std::min(21, player + 1);
    if (player > 21) return "Bust";
    if (player >= dealer) {
      p.AddMoney(bet * 2);
      return "BJ win";
    }
    return "BJ lose";
  }

  void StartAuction(const std::string& title, int start, bool container) {
    lot_ = {title, start, 12, container};
    activeAuction_ = true;
  }
  std::string TickAuction(Character& p, float dt) {
    if (!activeAuction_) return {};
    auctionAcc_ += dt;
    if (auctionAcc_ >= 1.f) {
      auctionAcc_ = 0;
      lot_.timer--;
      if (GetRandomValue(0, 100) < 40) lot_.price += GetRandomValue(1000, 8000);
      if (lot_.timer <= 0) {
        activeAuction_ = false;
        return "Auction ended at $" + std::to_string(lot_.price);
      }
    }
    if (IsKeyPressed(KEY_B) && p.Money() >= lot_.price + 5000) {
      lot_.price += 5000;
      lot_.timer = std::max(lot_.timer, 3);
      return "Your bid $" + std::to_string(lot_.price);
    }
    return {};
  }
  std::string BuyoutAuction(Character& p) {
    if (!activeAuction_) return "No auction";
    if (p.Money() < lot_.price) return "Can't afford";
    p.AddMoney(-lot_.price);
    activeAuction_ = false;
    if (lot_.container) {
      if (GetRandomValue(0, 100) < 50) {
        p.Inv().cars.push_back("Container Rare");
        factions.Add(FactionId::Elite, 2);
        return "Container: rare car!";
      }
      return "Container: junk";
    }
    p.Inv().cars.push_back(lot_.title);
    return "Won " + lot_.title;
  }

  void TickFence(float dt) {
    fence_.relocateTimer -= dt;
    if (fence_.relocateTimer <= 0) {
      fence_.relocateTimer = 90.f;
      fence_.pos = {(float)GetRandomValue(-40, 40), 0, (float)GetRandomValue(-40, 40)};
    }
  }
  std::string FenceSellCar(Character& p) {
    if (p.Inv().cars.empty()) return "No cars";
    std::string c = p.Inv().cars.back();
    p.Inv().cars.pop_back();
    int pay = 30000 + GetRandomValue(0, 40000);
    p.AddMoney(pay);
    p.AddHeat(10);
    factions.Add(FactionId::Syndicate, 2);
    factions.Add(FactionId::Police, -3);
    return "Fence bought " + c + " +$" + std::to_string(pay);
  }
  std::string FenceBuyIllegal(Character& p, int idx) {
    if (idx < 0 || idx >= (int)fence_.stock.size()) return "Bad item";
    if (p.Money() < 80000) return "Need $80k";
    p.AddMoney(-80000);
    p.Inv().materials[fence_.stock[idx]]++;
    p.AddHeat(5);
    factions.Add(FactionId::Syndicate, 1);
    return "Bought " + fence_.stock[idx];
  }
  FenceNPC& Fence() { return fence_; }

  std::string DarkNetContract(Character& p, int kind) {
    factions.Add(FactionId::Hackers, 1);
    if (kind == 0) return "Contract: steal marked car";
    if (kind == 1) {
      if (p.Money() < 25000) return "Hack $25k";
      p.AddMoney(-25000);
      factions.Add(FactionId::Hackers, 3);
      factions.Add(FactionId::Police, -4);
      return "Cameras down 60s";
    }
    if (p.Money() < 100000) return "Weapon $100k";
    p.AddMoney(-100000);
    p.Inv().weapons.push_back("Illegal SMG");
    p.AddHeat(15);
    factions.Add(FactionId::Syndicate, 2);
    return "DarkNet weapon delivered";
  }

  std::string TryArrest(Character& p, WorldAndEntities3D& w) {
    if (p.Wanted() < 3) return {};
    int chance = 30 + p.Wanted() * 10 - factions.Get(FactionId::Police) / 5;
    if (GetRandomValue(0, 100) > chance) return {};
    p.Inv().weapons.clear();
    w.ArrestPlayer(2.f + (float)p.Wanted());
    p.SetWanted(0);
    factions.Add(FactionId::Police, -5);
    return "ARRESTED — KPZ";
  }
  std::string Lawyer(Character& p, WorldAndEntities3D& w) {
    if (!w.Jail().occupied) return "Not in jail";
    if (p.Money() < 150000) return "Lawyer $150k";
    p.AddMoney(-150000);
    w.Jail().occupied = false;
    w.Jail().timer = 0;
    factions.Add(FactionId::Elite, 2);
    return "Released by lawyer";
  }
  std::string Escape(WorldAndEntities3D& w, Character& p) {
    if (!w.Jail().occupied) return "Not in jail";
    if (w.TryEscape()) {
      p.AddHeat(20);
      p.SetWanted(2);
      factions.Add(FactionId::Police, -10);
      factions.Add(FactionId::Syndicate, 3);
      return "Escaped KPZ!";
    }
    return "Escape failed — more time";
  }

  std::string DriftTournament(Character& p) {
    if (p.Energy() < 15) return "Need energy";
    p.SetEnergy(p.Energy() - 15);
    int score = GetRandomValue(40, 99);
    p.AddMoney(score * 500);
    factions.Add(FactionId::Racers, 3);
    return "Drift score " + std::to_string(score);
  }

private:
  struct Biz { std::string id; int staff; int income; int risk; };
  std::unordered_map<std::string, Biz> biz_;
  float day_ = 0;
  AuctionLot lot_{};
  bool activeAuction_ = false;
  float auctionAcc_ = 0;
  FenceNPC fence_;
};

class GameplayAndUI {
public:
  void Init(bool hasSave) {
    ui_.Init(hasSave);
    deep_ = {};
  }

  UIAndSystems& UI() { return ui_; }
  DeepSystems& Deep() { return deep_; }

  void Update(float dt, Character& p, WorldAndEntities3D& world) {
    if (dt < 0.f) dt = 0.f;
    if (dt > 0.1f) dt = 0.1f;
    ui_.Mech().Tick(p, dt);
    deep_.BusinessTick(p, dt);
    deep_.TickFence(dt);
    auto a = deep_.TickAuction(p, dt);
    if (!a.empty()) lastToast_ = a;
    auto ar = deep_.TryArrest(p, world);
    if (!ar.empty()) lastToast_ = ar;
    if (world.UpdateJail(dt)) lastToast_ = "Released from KPZ";
  }

  const std::string& Toast() const { return lastToast_; }
  void ClearToast() { lastToast_.clear(); }

private:
  UIAndSystems ui_;
  DeepSystems deep_;
  std::string lastToast_;
};
