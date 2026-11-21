#include "Character.hpp"
#include "Core/Config.hpp"
#include <cmath>

Character::Character() {
  skills_["stealth"] = 0;
  skills_["driving"] = 0;
  skills_["shooting"] = 0;
  skills_["hacking"] = 0;
  skills_["negotiation"] = 0;
  inv_.materials["scrap"] = 0;
  inv_.materials["electronics"] = 0;
  inv_.materials["chemicals"] = 0;
  inv_.materials["fabric"] = 0;
  inv_.materials["metal"] = 0;
}

void Character::Update(float dt) {
  animTime_ += dt;
}

void Character::Draw(float x, float y) {
  // simple procedural stick figure with breathing
  float breath = sinf(animTime_ * 2.2f) * 2.f;
  Color skin = {210, 170, 140, 255};
  Color shirt = {40, 40, 50, 255};
  Color pants = {25, 25, 35, 255};

  // shadow
  DrawEllipse((int)x, (int)(y + 52), 22, 8, Fade(BLACK, 0.35f));

  // legs
  DrawRectangleRounded({x - 10, y + 28, 8, 22}, 0.3f, 4, pants);
  DrawRectangleRounded({x + 2, y + 28, 8, 22}, 0.3f, 4, pants);
  // body
  float turn = cosf(facingAngle_);
  float bodyW = 18 + fabsf(turn) * 6;
  DrawRectangleRounded({x - bodyW/2, y + breath, bodyW, 30}, 0.25f, 6, shirt);
  // head
  DrawCircle((int)x, (int)(y - 14 + breath * 0.5f), 14, skin);
  // hair
  DrawCircle((int)x, (int)(y - 22 + breath * 0.5f), 12, hairColor_);
  // eyes follow mouse slightly
  Vector2 m = GetMousePosition();
  float dx = (m.x - x) * 0.01f;
  if (dx > 2) dx = 2; if (dx < -2) dx = -2;
  DrawCircle((int)(x - 4 + dx), (int)(y - 16 + breath * 0.5f), 2, BLACK);
  DrawCircle((int)(x + 4 + dx), (int)(y - 16 + breath * 0.5f), 2, BLACK);
}

void Character::AddHeat(int h) {
  heat_ += h;
  if (heat_ > 100) heat_ = 100;
  if (heat_ < 0) heat_ = 0;
  if (heat_ > 70 && wanted_ < 5 && GetRandomValue(0, 100) < 30)
    wanted_++;
}

void Character::AddStat(const std::string& s, int v) {
  if (s == "strength") strength_ += v;
  else if (s == "agility") agility_ += v;
  else if (s == "intelligence") intelligence_ += v;
}

int Character::Skill(const std::string& id) const {
  auto it = skills_.find(id);
  return it == skills_.end() ? 0 : it->second;
}

void Character::AddSkill(const std::string& id, int v) {
  skills_[id] += v;
}

std::vector<std::string> Character::AddExp(int amount) {
  std::vector<std::string> msgs;
  exp_ += amount;
  while (exp_ >= level_ * 140) {
    exp_ -= level_ * 140;
    level_++;
    skillPoints_++;
    money_ += 12000 + level_ * 800;
    msgs.push_back("LEVEL " + std::to_string(level_) + "!");
  }
  return msgs;
}

void Character::DrawCar(float x, float y) {
  if (activeCar_.empty()) return;
  Color body = {carR_, carG_, carB_, 255};
  // simple side/front based on facing
  float side = fabsf(sinf(facingAngle_));
  int w = (int)(70 + side * 20);
  int h = 28;
  DrawEllipse((int)x, (int)(y + 18), w/2 + 4, 8, Fade(BLACK, 0.35f));
  DrawRectangleRounded({x - w/2.f, y - h/2.f, (float)w, (float)h}, 0.3f, 6, body);
  DrawRectangleRounded({x - w/2.f + 10, y - h/2.f - 12, (float)w - 20, 14}, 0.2f, 4, body);
  // wheels
  DrawCircle((int)(x - w/3), (int)(y + h/2), 7, Color{20,20,20,255});
  DrawCircle((int)(x + w/3), (int)(y + h/2), 7, Color{20,20,20,255});
  if (neonCar_) {
    DrawEllipse((int)x, (int)(y + h/2 + 4), w/2, 6, Fade(Color{0, 220, 255, 255}, 0.45f));
  }
  if (turboLvl_ > 0) {
    DrawText("TURBO", (int)(x - 20), (int)(y - 36), 12, Color{244, 196, 48, 255});
  }
}
