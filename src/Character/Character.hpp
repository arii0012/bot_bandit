#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <map>
#include <cstdint>

struct Inventory {
  std::vector<std::string> clothes;
  std::vector<std::string> cars;
  std::vector<std::string> houses;
  std::vector<std::string> weapons;
  std::vector<std::string> gadgets;
  std::vector<std::string> items;
  std::map<std::string, int> materials; // scrap, electronics, ...
};

class Character {
public:
  Character();

  void Update(float dt);
  void Draw(float x, float y);
  void Draw3D(Vector3 pos, float scale = 1.f);
  void DrawCar(float x, float y);
  float& Facing() { return facingAngle_; }
  void SetFacing(float a) { facingAngle_ = a; }

  const std::string& Name() const { return name_; }
  void SetName(const std::string& n) { name_ = n; }
  int64_t Money() const { return money_; }
  void AddMoney(int64_t v) { money_ += v; if (money_ < 0) money_ = 0; }
  int Level() const { return level_; }
  int Exp() const { return exp_; }
  int Energy() const { return energy_; }
  void SetEnergy(int e) { energy_ = e; if (energy_ > 100) energy_ = 100; if (energy_ < 0) energy_ = 0; }
  int Heat() const { return heat_; }
  void AddHeat(int h);
  int Wanted() const { return wanted_; }
  void SetWanted(int w) { wanted_ = w; if (wanted_ < 0) wanted_ = 0; if (wanted_ > 5) wanted_ = 5; }
  int Strength() const { return strength_; }
  int Agility() const { return agility_; }
  int Intelligence() const { return intelligence_; }
  void AddStat(const std::string& s, int v);

  Inventory& Inv() { return inv_; }
  const Inventory& Inv() const { return inv_; }

  std::string CityId() const { return cityId_; }
  void SetCity(const std::string& id) { cityId_ = id; }

  std::string ActiveCar() const { return activeCar_; }
  void SetActiveCar(const std::string& c) { activeCar_ = c; }

  bool GenderFemale() const { return female_; }
  void SetFemale(bool f) { female_ = f; }
  int HairStyle() const { return hairStyle_; }
  void SetHairStyle(int h) { hairStyle_ = h; }
  Color HairColor() const { return hairColor_; }
  void SetHairColor(Color c) { hairColor_ = c; }

  int Skill(const std::string& id) const;
  void AddSkill(const std::string& id, int v);
  int SkillPoints() const { return skillPoints_; }
  void AddSkillPoints(int v) { skillPoints_ += v; }

  std::vector<std::string> AddExp(int amount); // level-up messages

  // serialization helpers
  std::string name_ = "Бандит";
  int64_t money_ = 45000;
  int level_ = 1;
  int exp_ = 0;
  int energy_ = 100;
  int heat_ = 0;
  int wanted_ = 0;
  int strength_ = 1, agility_ = 1, intelligence_ = 1;
  int skillPoints_ = 0;
  std::map<std::string, int> skills_;
  Inventory inv_;
  std::string cityId_ = "липецк";
  std::string activeCar_;
  bool female_ = false;
  Color hairColor_ = {40, 30, 20, 255};
  Color colorEyes_ = {40, 80, 140, 255};
  Color colorTop_ = {48, 50, 68, 255};
  Color colorPants_ = {30, 30, 42, 255};
  Color colorHat_ = {40, 40, 48, 255};
  Color colorShoes_ = {22, 22, 28, 255};
  Color colorAcc_ = {220, 180, 40, 255};
  int bodyType_ = 1; // 0 thin 1 athlete 2 heavy
  int hairStyle_ = 0;
  int topStyle_ = 0;
  int pantsStyle_ = 0;
  int hatStyle_ = 0;
  int shoeStyle_ = 0;
  int accStyle_ = 0;
  Model mesh_{};
  bool meshLoaded_ = false;
  float animTime_ = 0.f;
  float facingAngle_ = 0.f; // radians, 0 = front
  int engineLvl_ = 0, turboLvl_ = 0;
  bool neonCar_ = false;
  unsigned char carR_ = 200, carG_ = 40, carB_ = 40;
  float dirt_ = 0.f;
  float wet_ = 0.f;
  float blood_ = 0.f;
  float lip_ = 0.f; // lipsync 0..1
  void SetDirt(float v) { dirt_ = v < 0 ? 0 : (v > 1 ? 1 : v); }
  void SetWet(float v) { wet_ = v < 0 ? 0 : (v > 1 ? 1 : v); }
  void SetBlood(float v) { blood_ = v < 0 ? 0 : (v > 1 ? 1 : v); }
  void SetLip(float v) { lip_ = v < 0 ? 0 : (v > 1 ? 1 : v); }
  float Dirt() const { return dirt_; }
  float Wet() const { return wet_; }
};
