#pragma once
#include <string>
#include <algorithm>

class Character;

// Body + vehicle damage
struct BodyDamage {
  float hp = 100.f;       // 0..100 player vitality under energy
  float bleed = 0.f;      // DoT
  float limp = 0.f;       // speed penalty 0..1
  bool knocked = false;
};

struct VehicleDamage {
  float body = 100.f;     // 0..100
  float engine = 100.f;
  float tires = 100.f;
  float visual = 0.f;     // dents 0..1 for draw
  bool totaled = false;
};

class DamageSystem {
public:
  static BodyDamage& Body(Character& p);
  static VehicleDamage& Car(Character& p);

  static void HitPlayer(Character& p, float amount, const std::string& reason, std::string& msg);
  static void HealPlayer(Character& p, float amount, std::string& msg);
  static void Tick(Character& p, float dt, std::string& outEvent);

  static void CrashCar(Character& p, float severity, std::string& msg); // 0..1
  static void RepairCar(Character& p, std::string& msg);
  static float PowerMult(const Character& p); // engine damage slows car
};
