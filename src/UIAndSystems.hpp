#pragma once
#include "UITheme.hpp"
#include "CustomCursor.hpp"
#include "MainMenu.hpp"
#include "Garage3D.hpp"
#include "Mechanics20.hpp"
#include "StoryAndCutscenes.hpp"
#include "ModelsAndWorld3D.hpp"
#include "EngineCore.hpp"
#include "Character/Character.hpp"
#include "Core/FontUI.hpp"
#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <cstdlib>

// Extra 20 mechanics on top of Mechanics20
class Mechanics40 : public Mechanics20 {
public:
  // --- 10 gameplay ---
  std::string BlackMarketTune(Character& p) {
    if (p.Money() < 150000) return "Black market $150k";
    p.AddMoney(-150000);
    p.engineLvl_ = std::min(10, p.engineLvl_ + 2);
    p.turboLvl_ = std::min(5, p.turboLvl_ + 1);
    p.Inv().materials["nos"]++;
    p.Inv().materials["bulletproof_glass"]++;
    return "Legendary tune installed";
  }
  std::string TandemDrift(Character& p) {
    if (p.Energy() < 15) return "Need energy";
    p.SetEnergy(p.Energy() - 15);
    int score = GetRandomValue(40, 99);
    driftPoints_ += score;
    p.AddMoney(score * 500);
    return "Tandem drift score " + std::to_string(score);
  }
  std::string ArmoredHeist(Character& p) {
    if (p.Energy() < 40) return "Need energy 40";
    p.SetEnergy(p.Energy() - 40);
    if (GetRandomValue(0, 100) < 35 + p.Skill("stealth")) {
      int g = 200000 + GetRandomValue(0, 150000);
      p.AddMoney(g);
      p.AddHeat(25);
      return "Vault hit +$" + std::to_string(g);
    }
    p.SetWanted(std::min(5, p.Wanted() + 3));
    return "Heist failed — wanted";
  }
  std::string CyberHack(Character& p, int mode) {
    // 0 traffic 1 cameras 2 lights
    if (p.Energy() < 5) return "No energy";
    p.SetEnergy(p.Energy() - 5);
    if (mode == 0) { trafficHack_ = 20.f; return "Traffic lights hacked 20s"; }
    if (mode == 1) { camerasOff_ = 30.f; return "Cameras offline 30s"; }
    lightsOff_ = 15.f; return "Street lights off 15s";
  }
  std::string CollectVinyl(Character& p) {
    if (GetRandomValue(0, 100) < 25) {
      vinyls_++;
      prestige_ += 5;
      return "Rare vinyl! total " + std::to_string(vinyls_);
    }
    return "No vinyl this time";
  }
  std::string NightCinema(Character& p) {
    if (p.Money() < 3000) return "Cinema $3k";
    p.AddMoney(-3000);
    p.SetEnergy(std::min(100, p.Energy() + 30));
    buffTimer_ = 120.f;
    return "Cinema buff 2 min";
  }
  std::string TreasureHunt(Character& p) {
    if (GetRandomValue(0, 100) < 40) {
      int g = 10000 + GetRandomValue(0, 40000);
      p.AddMoney(g);
      p.Inv().clothes.push_back("Rare fit");
      return "Stash found +$" + std::to_string(g);
    }
    return "Empty coordinates";
  }
  std::string BuyHeli(Character& p) {
    if (p.Money() < 5000000) return "Heli $5M";
    p.AddMoney(-5000000);
    hasHeli_ = true;
    return "Helipad unlocked";
  }
  std::string BuyYacht(Character& p) {
    if (p.Money() < 8000000) return "Yacht $8M";
    p.AddMoney(-8000000);
    hasYacht_ = true;
    return "Yacht owned";
  }
  std::string BountyHunt(Character& p) {
    if (p.Energy() < 20) return "Need energy";
    p.SetEnergy(p.Energy() - 20);
    if (GetRandomValue(0, 100) < 50 + p.Skill("combat")) {
      int g = 80000 + GetRandomValue(0, 70000);
      p.AddMoney(g);
      return "Bounty complete +$" + std::to_string(g);
    }
    p.AddHeat(10);
    return "Target escaped";
  }
  std::string RandomStreetEvent(Character& p) {
    int e = GetRandomValue(0, 3);
    if (e == 0) { p.AddMoney(5000); return "Helped broken NPC car +$5k"; }
    if (e == 1) { p.SetWanted(std::min(5, p.Wanted() + 1)); return "Police chase nearby — heat"; }
    if (e == 2) { p.SetEnergy(std::max(0, p.Energy() - 10)); return "Street fight — bruised"; }
    prestige_ += 2; return "Crowd applause +prestige";
  }

  // --- 10 realistic ---
  std::string Refuel(Character& p, int grade) {
    // 0=92 1=95 2=100 3=electric
    float need = 100.f - fuel_;
    if (need < 1.f) return "Tank full";
    int price = (int)(need * (grade == 0 ? 50 : grade == 1 ? 60 : grade == 2 ? 75 : 40));
    if (p.Money() < price) return "Need $" + std::to_string(price);
    p.AddMoney(-price);
    fuel_ = 100.f;
    fuelGrade_ = grade;
    return "Refueled grade " + std::to_string(grade);
  }
  void DriveFuel(float speed, float dt) {
    float mult = fuelGrade_ == 3 ? 0.5f : (fuelGrade_ == 2 ? 0.9f : 1.1f);
    fuel_ = std::max(0.f, fuel_ - speed * 0.002f * mult * dt);
  }
  std::string CarWash(Character& p) {
    if (p.Money() < 2000) return "Wash $2k";
    p.AddMoney(-2000);
    dirt_ = 0.f;
    return "Detailing done";
  }
  void AddDirt(float d) { dirt_ = std::min(100.f, dirt_ + d); }
  std::string BuyInsurance(Character& p, bool kasko) {
    int cost = kasko ? 50000 : 15000;
    if (p.Money() < cost) return "Insurance $" + std::to_string(cost);
    p.AddMoney(-cost);
    if (kasko) kasko_ = true; else osago_ = true;
    return kasko ? "KASKO active" : "OSAGO active";
  }
  std::string SpeedFine(Character& p, float speed) {
    if (speed < 80.f) return {};
    int fine = (int)((speed - 80.f) * 200.f);
    if (camerasOff_ > 0) return "Camera offline — no fine";
    p.AddMoney(-fine);
    return "Speed camera -$" + std::to_string(fine);
  }
  void DailyBills(Character& p) {
    int bill = 500 + (int)p.Inv().cars.size() * 200;
    if (p.Money() >= bill) p.AddMoney(-bill);
    else creditScore_ = std::max(0, creditScore_ - 5);
  }
  std::string OrderFood(Character& p) {
    if (p.Money() < 1500) return "Food $1.5k";
    p.AddMoney(-1500);
    fridge_ = std::min(10, fridge_ + 3);
    return Eat(p);
  }
  std::string Barber(Character& p, int style) {
    if (p.Money() < 5000) return "Barber $5k";
    p.AddMoney(-5000);
    p.SetHairStyle(style % 8);
    return "New haircut";
  }
  std::string Tattoo(Character& p) {
    if (p.Money() < 12000) return "Tattoo $12k";
    p.AddMoney(-12000);
    tattoos_++;
    return "Tattoo #" + std::to_string(tattoos_);
  }
  std::string ImpoundRetrieve(Character& p) {
    if (!impounded_) return "No car in impound";
    if (p.Money() < 25000) return "Impound $25k";
    p.AddMoney(-25000);
    impounded_ = false;
    return "Car retrieved";
  }
  void ImpoundCar() { impounded_ = true; }
  std::string BankDeposit(Character& p, int amount) {
    if (amount <= 0 || p.Money() < amount) return "Bad amount";
    p.AddMoney(-amount);
    deposit_ += amount;
    creditScore_ = std::min(100, creditScore_ + 2);
    return "Deposit $" + std::to_string(amount);
  }
  std::string ChangeTires(Character& p, int type) {
    // 0 summer 1 winter 2 slicks
    int cost = type == 2 ? 40000 : 15000;
    if (p.Money() < cost) return "Tires $" + std::to_string(cost);
    p.AddMoney(-cost);
    tireType_ = type;
    tireWear_ = 0.f;
    return "Tires set";
  }
  void WearTires(float slip, float dt) {
    tireWear_ = std::min(100.f, tireWear_ + slip * 10.f * dt);
  }
  std::string MedKitTrunk(Character& p) {
    if (p.Inv().items.empty()) return "No medkit";
    p.SetEnergy(std::min(100, p.Energy() + 40));
    return "Used trunk medkit";
  }
  std::string Hospital(Character& p) {
    int cost = medInsurance_ ? 5000 : 25000;
    if (p.Money() < cost) return "Hospital $" + std::to_string(cost);
    p.AddMoney(-cost);
    p.SetEnergy(100);
    return "Treated";
  }
  std::string BuyMedInsurance(Character& p) {
    if (p.Money() < 30000) return "Med insurance $30k";
    p.AddMoney(-30000);
    medInsurance_ = true;
    return "Medical insurance ON";
  }

  void Tick(Character& p, float dt) {
    Mechanics20::NeedsTick(p, dt);
    Mechanics20::MiningTick(p, dt);
    if (trafficHack_ > 0) trafficHack_ -= dt;
    if (camerasOff_ > 0) camerasOff_ -= dt;
    if (lightsOff_ > 0) lightsOff_ -= dt;
    if (buffTimer_ > 0) buffTimer_ -= dt;
    dayTimer_ += dt;
    if (dayTimer_ > 120.f) { dayTimer_ = 0; DailyBills(p); }
  }

  float Fuel() const { return fuel_; }
  float Dirt() const { return dirt_; }
  int Prestige() const { return prestige_; }
  int Credit() const { return creditScore_; }
  bool HasHeli() const { return hasHeli_; }
  bool HasYacht() const { return hasYacht_; }

private:
  int driftPoints_ = 0, vinyls_ = 0, prestige_ = 0, tattoos_ = 0;
  float trafficHack_ = 0, camerasOff_ = 0, lightsOff_ = 0, buffTimer_ = 0;
  bool hasHeli_ = false, hasYacht_ = false;
  float fuel_ = 70.f;
  int fuelGrade_ = 1;
  float dirt_ = 0.f;
  bool osago_ = false, kasko_ = false, medInsurance_ = false;
  int creditScore_ = 50;
  int fridge_ = 2;
  bool impounded_ = false;
  int deposit_ = 0;
  int tireType_ = 0;
  float tireWear_ = 0.f;
  float dayTimer_ = 0.f;
};

class SmartphoneUI {
public:
  void Open() { open_ = true; page_ = 0; }
  void Close() { open_ = false; }
  bool IsOpen() const { return open_; }
  void Update(float) {
    if (!open_) return;
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) open_ = false;
    if (IsKeyPressed(KEY_RIGHT)) page_ = (page_ + 1) % 8;
    if (IsKeyPressed(KEY_LEFT)) page_ = (page_ + 7) % 8;
  }
  void Draw(int sw, int sh, Mechanics40& m, Character& p) {
    if (!open_) return;
    float w = 320, h = 560;
    float x = sw - w - 24, y = (sh - h) * 0.5f;
    UITheme::GlassPanel({x, y, w, h}, 0.1f);
    const char* apps[8] = {"Bank","Crypto","DarkNet","Taxi","Insta","Estate","GPS","Radio"};
    UITheme::Title(apps[page_], x + 20, y + 16, 22);
    UITheme::Caption("←/→ apps  P close", x + 20, y + 48, 14);
    char buf[128];
    snprintf(buf, sizeof(buf), "Cash $%lld  Crypto %.1f", (long long)p.Money(), m.Crypto());
    UITheme::Caption(buf, x + 20, y + 80, 16);
    snprintf(buf, sizeof(buf), "Fuel %.0f%% Dirt %.0f Credit %d", m.Fuel(), m.Dirt(), m.Credit());
    UITheme::Caption(buf, x + 20, y + 110, 16);
  }
private:
  bool open_ = false;
  int page_ = 0;
};

class DevConsoleExt {
public:
  void Open() { open_ = true; }
  void Close() { open_ = false; }
  bool IsOpen() const { return open_; }
  void Toggle() { open_ = !open_; }

  std::string Handle(const std::string& line, Character& p, Mechanics40& m, StoryAndCutscenes& story) {
    if (line == "/help")
      return "cmds: give_money spawn_car godmode wanted unlock_all play_cutscene refuel wash heist cyber exit";
    if (line.rfind("/give_money", 0) == 0) { p.AddMoney(100000); return "+100k"; }
    if (line == "/spawn_car") { p.Inv().cars.push_back("Console Car"); return "car"; }
    if (line == "/godmode") { p.SetEnergy(100); p.SetWanted(0); return "ok"; }
    if (line.rfind("/wanted", 0) == 0) { p.SetWanted(3); return "wanted 3"; }
    if (line == "/unlock_all") { p.AddMoney(99999999); return "unlock"; }
    if (line.rfind("/play_cutscene", 0) == 0) { story.Play("debut"); return "cutscene"; }
    if (line == "/refuel") return m.Refuel(p, 1);
    if (line == "/wash") return m.CarWash(p);
    if (line == "/heist") return m.ArmoredHeist(p);
    if (line == "/cyber") return m.CyberHack(p, 0);
    if (line == "/exit") return "EXIT";
    return "unknown";
  }

  void Draw(int sw, int sh) {
    if (!open_) return;
    DrawRectangle(0, sh - 40, sw, 40, Fade(BLACK, 0.85f));
    DrawText(("> " + input_).c_str(), 12, sh - 28, 20, GREEN);
  }
  void Char(int c) {
    if (c >= 32 && c < 127 && input_.size() < 120) input_.push_back((char)c);
  }
  void Backspace() { if (!input_.empty()) input_.pop_back(); }
  std::string TakeEnter() {
    std::string s = input_;
    input_.clear();
    return s;
  }
private:
  bool open_ = false;
  std::string input_;
};

class UIAndSystems {
public:
  void Init(bool hasSave) {
    menu_.Open(hasSave);
    cursor_.Init();
    world_.Init();
  }

  MainMenu& Menu() { return menu_; }
  CustomCursor& Cursor() { return cursor_; }
  Garage3D& Garage() { return garage_; }
  Mechanics40& Mech() { return mech_; }
  StoryAndCutscenes& Story() { return story_; }
  ModelsAndWorld3D& World3D() { return world_; }
  SmartphoneUI& Phone() { return phone_; }
  DevConsoleExt& Console() { return console_; }
  EngineOptimizer& Opt() { return opt_; }
  ParticleSystem3D& Fx() { return fx_; }
  PhysicsWorld& Phys() { return phys_; }

private:
  MainMenu menu_;
  CustomCursor cursor_;
  Garage3D garage_;
  Mechanics40 mech_;
  StoryAndCutscenes story_;
  ModelsAndWorld3D world_;
  SmartphoneUI phone_;
  DevConsoleExt console_;
  EngineOptimizer opt_;
  ParticleSystem3D fx_;
  PhysicsWorld phys_;
};
