#pragma once
#include "UIAndSystems.hpp"
#include "CoreEngine.hpp"
#include "WorldAndEntities3D.hpp"
#include "Character/Character.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_map>

struct AuctionLot {
  std::string title;
  int price = 0;
  int timer = 15;
  bool container = false;
};

struct FenceNPC {
  std::string name = "Барыга";
  Vector3 pos{12, 0, -18};
  float relocateTimer = 90.f;
  std::vector<std::string> stock = {"twin_turbo", "nos", "fake_passport", "vin_kit"};
};

class DeepSystems {
public:
  // Business
  std::string BuyBusiness(Character& p, const std::string& id, int price) {
    if (p.Money() < price) return "Need $" + std::to_string(price);
    p.AddMoney(-price);
    biz_[id] = {id, 1, 5000, 10};
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
      int gain = b.income * b.staff;
      int tax = gain / 10;
      int raid = GetRandomValue(0, 100);
      if (raid < b.risk) {
        p.AddHeat(8);
        continue;
      }
      p.AddMoney(gain - tax);
    }
  }

  // Casino deep
  std::string Roulette(Character& p, int bet, int number) {
    if (p.Money() < bet) return "No bet money";
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
    if (p.Money() < bet) return "No bet";
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

  // Auction
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
        return "Container: rare car!";
      }
      return "Container: junk";
    }
    p.Inv().cars.push_back(lot_.title);
    return "Won " + lot_.title;
  }

  // Fence / барыга
  void TickFence(float dt) {
    fence_.relocateTimer -= dt;
    if (fence_.relocateTimer <= 0) {
      fence_.relocateTimer = 90.f;
      fence_.pos = {GetRandomValue(-40, 40) * 1.f, 0, GetRandomValue(-40, 40) * 1.f};
    }
  }
  std::string FenceSellCar(Character& p) {
    if (p.Inv().cars.empty()) return "No cars";
    std::string c = p.Inv().cars.back();
    p.Inv().cars.pop_back();
    int pay = 30000 + GetRandomValue(0, 40000);
    p.AddMoney(pay);
    p.AddHeat(10);
    return "Fence bought " + c + " +$" + std::to_string(pay);
  }
  std::string FenceBuyIllegal(Character& p, int idx) {
    if (idx < 0 || idx >= (int)fence_.stock.size()) return "Bad item";
    if (p.Money() < 80000) return "Need $80k";
    p.AddMoney(-80000);
    p.Inv().materials[fence_.stock[idx]]++;
    p.AddHeat(5);
    return "Bought " + fence_.stock[idx];
  }
  FenceNPC& Fence() { return fence_; }

  // DarkNet
  std::string DarkNetContract(Character& p, int kind) {
    // 0 steal 1 hack cams 2 weapon
    if (kind == 0) return "Contract: steal marked car";
    if (kind == 1) {
      if (p.Money() < 25000) return "Hack $25k";
      p.AddMoney(-25000);
      return "Cameras down 60s (pay hacker)";
    }
    if (p.Money() < 100000) return "Weapon $100k";
    p.AddMoney(-100000);
    p.Inv().weapons.push_back("Illegal SMG");
    p.AddHeat(15);
    return "DarkNet weapon delivered";
  }

  // Arrest / jail flow
  std::string TryArrest(Character& p, WorldAndEntities3D& w) {
    if (p.Wanted() < 3) return {};
    if (GetRandomValue(0, 100) > 30 + p.Wanted() * 10) return {};
    // confiscate illegal
    p.Inv().weapons.clear();
    w.ArrestPlayer(2.f + p.Wanted());
    p.SetWanted(0);
    return "ARRESTED — KPZ";
  }
  std::string Lawyer(Character& p, WorldAndEntities3D& w) {
    if (!w.Jail().occupied) return "Not in jail";
    if (p.Money() < 150000) return "Lawyer $150k";
    p.AddMoney(-150000);
    w.Jail().occupied = false;
    return "Released by lawyer";
  }
  std::string BribeChief(Character& p, WorldAndEntities3D& w) {
    if (!w.Jail().occupied) return "Not in jail";
    if (p.Money() < 200000) return "Bribe $200k";
    p.AddMoney(-200000);
    w.Jail().occupied = false;
    p.AddHeat(5);
    return "Bribed out";
  }
  std::string Escape(WorldAndEntities3D& w, Character& p) {
    if (!w.Jail().occupied) return "Not in jail";
    if (w.TryEscape()) {
      p.AddHeat(20);
      p.SetWanted(2);
      return "Escaped KPZ!";
    }
    return "Escape failed — more time";
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
