#include "Character.hpp"
#include "Core/FontUI.hpp"
#include "Core/Config.hpp"
#include <cmath>
#include <algorithm>

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

static Color Shade(Color c, float k) {
  auto ch = [](unsigned char v, float k) -> unsigned char {
    return (unsigned char)std::clamp((int)(v * k), 0, 255);
  };
  return Color{ch(c.r, k), ch(c.g, k), ch(c.b, k), c.a};
}

void Character::Draw(float x, float y) {
  const float S = 2.1f;
  float t = animTime_;
  float breath = sinf(t * 2.1f) * 2.8f;
  float ang = facingAngle_;
  float turn = cosf(ang);
  float side = sinf(ang);
  float walk = sinf(t * 9.f);
  float walkC = cosf(t * 9.f);
  bool moving = fabsf(side) > 0.15f || fabsf(walk) > 0.05f;
  float lean = moving ? -3.f * S : 0.f;

  Color skin = female_ ? Color{232, 186, 162, 255} : Color{210, 168, 140, 255};
  auto shade = [](Color c, float m) -> Color {
    return Color{
      (unsigned char)std::min(255, (int)(c.r * m)),
      (unsigned char)std::min(255, (int)(c.g * m)),
      (unsigned char)std::min(255, (int)(c.b * m)),
      c.a};
  };
  Color skinD = shade(skin, 0.72f);
  Color shirt = {48, 50, 68, 255};
  Color pants = {30, 30, 42, 255};
  Color shoes = {22, 22, 28, 255};
  bool hoodie = false, jacket = false, suit = false, glasses = false, chain = false, bandana = false;
  for (auto& c : inv_.clothes) {
    if (c.find("hoodie") != std::string::npos || c.find("hudi") != std::string::npos) {
      shirt = {36, 42, 120, 255}; hoodie = true;
    }
    if (c.find("jacket") != std::string::npos || c.find("leather") != std::string::npos || c.find("kurt") != std::string::npos) {
      shirt = {28, 28, 32, 255}; jacket = true;
    }
    if (c.find("suit") != std::string::npos || c.find("kostyum") != std::string::npos) {
      shirt = {24, 24, 32, 255}; suit = true; pants = {20, 20, 28, 255};
    }
    if (c.find("sport") != std::string::npos) shirt = {190, 45, 50, 255};
    if (c.find("glasses") != std::string::npos || c.find("ochki") != std::string::npos) glasses = true;
    if (c.find("chain") != std::string::npos || c.find("tsep") != std::string::npos) chain = true;
    if (c.find("bandana") != std::string::npos) bandana = true;
  }

  // ground shadow
  DrawEllipse((int)x, (int)(y + 58 * S), (int)(34 * S), (int)(12 * S), Fade(BLACK, 0.4f));

  float legL = walk * 10.f * S * (moving ? 1.f : 0.25f);
  float legR = -walk * 10.f * S * (moving ? 1.f : 0.25f);
  float hipY = y + 8 * S + breath * 0.15f;

  // legs (thigh + shin + shoe)
  DrawRectangleRounded(Rectangle{x - 14 * S + legL, hipY, 12 * S, 22 * S}, 0.35f, 6, pants);
  DrawRectangleRounded(Rectangle{x + 2 * S + legR, hipY, 12 * S, 22 * S}, 0.35f, 6, pants);
  DrawRectangleRounded(Rectangle{x - 13 * S + legL * 1.1f, hipY + 20 * S, 11 * S, 20 * S}, 0.3f, 4, shade(pants, 0.85f));
  DrawRectangleRounded(Rectangle{x + 3 * S + legR * 1.1f, hipY + 20 * S, 11 * S, 20 * S}, 0.3f, 4, shade(pants, 0.85f));
  DrawRectangleRounded(Rectangle{x - 14 * S + legL * 1.15f, hipY + 38 * S, 14 * S, 9 * S}, 0.4f, 4, shoes);
  DrawRectangleRounded(Rectangle{x + 1 * S + legR * 1.15f, hipY + 38 * S, 14 * S, 9 * S}, 0.4f, 4, shoes);
  DrawRectangleRec(Rectangle{x - 14 * S + legL * 1.15f, hipY + 44 * S, 14 * S, 3 * S}, Color{244, 196, 48, 180});

  // hips / lower torso
  float bodyW = (22.f + fabsf(turn) * 6.f) * S;
  DrawRectangleRounded(Rectangle{x - bodyW * 0.5f, y - 6 * S + breath * 0.2f, bodyW, 18 * S}, 0.25f, 6, pants);

  // torso
  float torsoY = y - 34 * S + breath + lean;
  DrawRectangleRounded(Rectangle{x - bodyW * 0.55f + side * 2.f, torsoY, bodyW * 1.05f, 36 * S}, 0.22f, 8, shirt);
  DrawRectangleRounded(Rectangle{x - bodyW * 0.55f - 3 * S, torsoY + 2 * S, 6 * S, 32 * S}, 0.2f, 4, shade(shirt, 0.65f));
  if (hoodie) {
    DrawRectangleRounded(Rectangle{x - bodyW * 0.35f, torsoY - 6 * S, bodyW * 0.7f, 10 * S}, 0.4f, 6, shade(shirt, 0.8f));
  }
  if (jacket) {
    DrawRectangleRounded(Rectangle{x - bodyW * 0.6f, torsoY, 8 * S, 34 * S}, 0.15f, 4, Color{50, 40, 35, 255});
    DrawRectangleRounded(Rectangle{x + bodyW * 0.35f, torsoY, 8 * S, 34 * S}, 0.15f, 4, Color{50, 40, 35, 255});
  }
  if (suit) {
    DrawRectangleRec(Rectangle{x - 2 * S, torsoY + 4 * S, 4 * S, 22 * S}, Color{180, 40, 50, 255});
  }
  if (chain) {
    DrawCircleLines((int)x, (int)(torsoY + 14 * S), (int)(9 * S), Color{244, 196, 48, 255});
    DrawCircle((int)x, (int)(torsoY + 22 * S), (int)(3 * S), Color{244, 196, 48, 255});
  }

  // arms
  float armL = -walk * 8.f * S;
  float armR = walk * 8.f * S;
  DrawRectangleRounded(Rectangle{x - bodyW * 0.65f - 4 * S, torsoY + 4 * S + armL, 9 * S, 16 * S}, 0.4f, 4, shirt);
  DrawRectangleRounded(Rectangle{x + bodyW * 0.45f, torsoY + 4 * S + armR, 9 * S, 16 * S}, 0.4f, 4, shirt);
  DrawRectangleRounded(Rectangle{x - bodyW * 0.68f - 3 * S, torsoY + 18 * S + armL, 8 * S, 14 * S}, 0.4f, 4, skin);
  DrawRectangleRounded(Rectangle{x + bodyW * 0.48f, torsoY + 18 * S + armR, 8 * S, 14 * S}, 0.4f, 4, skin);
  // hands
  DrawCircle((int)(x - bodyW * 0.62f), (int)(torsoY + 34 * S + armL), (int)(4.5f * S), skin);
  DrawCircle((int)(x + bodyW * 0.55f), (int)(torsoY + 34 * S + armR), (int)(4.5f * S), skin);

  // neck + head
  float hx = x + side * 3.f * S;
  float hy = torsoY - 16 * S;
  DrawRectangleRounded(Rectangle{hx - 5 * S, hy + 8 * S, 10 * S, 10 * S}, 0.3f, 4, skin);
  DrawCircle((int)hx, (int)hy, (int)(15 * S), skin);
  DrawCircle((int)(hx - 4 * S), (int)(hy + 3 * S), (int)(12 * S), Fade(skinD, 0.4f));
  // chin
  DrawEllipse((int)hx, (int)(hy + 10 * S), (int)(8 * S), (int)(5 * S), skin);

  // eyes + blink
  float blink = (sinf(t * 1.6f) > 0.93f) ? 1.2f * S : 4.2f * S;
  DrawRectangleRounded(Rectangle{hx - 9 * S, hy - 2 * S, 7 * S, blink}, 0.4f, 3, WHITE);
  DrawRectangleRounded(Rectangle{hx + 2 * S, hy - 2 * S, 7 * S, blink}, 0.4f, 3, WHITE);
  if (blink > 2.f * S) {
    DrawCircle((int)(hx - 5.5f * S), (int)hy, (int)(2.2f * S), Color{30, 40, 55, 255});
    DrawCircle((int)(hx + 5.5f * S), (int)hy, (int)(2.2f * S), Color{30, 40, 55, 255});
    DrawCircle((int)(hx - 4.8f * S), (int)(hy - 0.8f * S), (int)(0.9f * S), WHITE);
    DrawCircle((int)(hx + 6.2f * S), (int)(hy - 0.8f * S), (int)(0.9f * S), WHITE);
  }
  // brows / mouth
  DrawRectangleRec(Rectangle{hx - 9 * S, hy - 6 * S, 7 * S, 1.5f * S}, shade(hairColor_, 0.7f));
  DrawRectangleRec(Rectangle{hx + 2 * S, hy - 6 * S, 7 * S, 1.5f * S}, shade(hairColor_, 0.7f));
  DrawEllipse((int)hx, (int)(hy + 7 * S), (int)(4 * S), (int)(1.6f * S), shade(skin, 0.75f));

  // hair
  Color hair = hairColor_;
  DrawCircle((int)hx, (int)(hy - 7 * S), (int)(14 * S), hair);
  if (hairStyle_ % 5 == 1) {
    DrawCircle((int)(hx - 12 * S), (int)hy, (int)(7 * S), hair);
    DrawCircle((int)(hx + 12 * S), (int)hy, (int)(7 * S), hair);
  } else if (hairStyle_ % 5 == 2) {
    DrawRectangleRounded(Rectangle{hx - 6 * S, hy - 20 * S, 12 * S, 12 * S}, 0.3f, 4, hair);
  } else if (hairStyle_ % 5 == 3 || female_) {
    DrawCircle((int)(hx - 11 * S), (int)(hy + 8 * S), (int)(8 * S), hair);
    DrawCircle((int)(hx + 11 * S), (int)(hy + 8 * S), (int)(8 * S), hair);
    DrawCircle((int)(hx - 8 * S), (int)(hy + 16 * S), (int)(6 * S), hair);
    DrawCircle((int)(hx + 8 * S), (int)(hy + 16 * S), (int)(6 * S), hair);
  } else if (hairStyle_ % 5 == 4) {
    for (int i = -2; i <= 2; ++i)
      DrawRectangleRec(Rectangle{hx + i * 4 * S - 2 * S, hy - 18 * S, 3 * S, 10 * S}, hair);
  }
  if (bandana) {
    DrawRectangleRounded(Rectangle{hx - 14 * S, hy - 4 * S, 28 * S, 7 * S}, 0.2f, 3, Color{190, 35, 40, 255});
  }
  if (glasses) {
    DrawRectangleRounded(Rectangle{hx - 12 * S, hy - 4 * S, 10 * S, 6 * S}, 0.2f, 2, Color{15, 15, 20, 230});
    DrawRectangleRounded(Rectangle{hx + 2 * S, hy - 4 * S, 10 * S, 6 * S}, 0.2f, 2, Color{15, 15, 20, 230});
    DrawLineEx(Vector2{hx - 2 * S, hy - 1 * S}, Vector2{hx + 2 * S, hy - 1 * S}, 2.f, Color{40, 40, 48, 255});
  }

  // weapon
  if (!inv_.weapons.empty()) {
    DrawRectangleRounded(Rectangle{x + bodyW * 0.5f, torsoY + 18 * S + armR, 7 * S, 24 * S}, 0.15f, 3, Color{45, 45, 52, 255});
    DrawRectangleRec(Rectangle{x + bodyW * 0.5f + 1 * S, torsoY + 16 * S + armR, 5 * S, 4 * S}, Color{30, 30, 35, 255});
  }

  // rim light
  DrawCircleLines((int)hx, (int)hy, (int)(16.5f * S), Fade(Color{0, 210, 255, 255}, 0.28f));
  DrawCircleLines((int)x, (int)(y - 4 * S), (int)(26 * S), Fade(Color{220, 40, 80, 255}, 0.12f));
  if (dirt_ > 0.05f) {
    DrawEllipse((int)x, (int)(y + 20 * S), (int)(18 * S * dirt_), (int)(8 * S), Fade(Color{60, 45, 30, 255}, dirt_ * 0.5f));
  }
  if (wet_ > 0.05f) {
    DrawCircleLines((int)hx, (int)hy, (int)(16 * S), Fade(Color{120, 160, 220, 255}, wet_ * 0.4f));
  }
  if (blood_ > 0.05f) {
    DrawCircle((int)(x + 8 * S), (int)(y - 10 * S), (int)(3 * S * blood_ + 1), Fade(Color{140, 20, 30, 255}, blood_));
  }
  if (lip_ > 0.05f) {
    DrawEllipse((int)hx, (int)(hy + 7 * S), (int)(4 * S), (int)(1.6f * S + lip_ * 3.f * S), shade(skin, 0.6f));
  }
}


void Character::Draw3D(Vector3 pos, float scale) {
  float t = animTime_;
  float breath = sinf(t * 2.1f) * 0.03f * scale;
  float walk = sinf(t * 9.f) * 0.15f * scale;
  Color skin = female_ ? Color{232, 186, 162, 255} : Color{210, 168, 140, 255};
  Color shirt = {48, 50, 68, 255};
  Color pants = {30, 30, 42, 255};
  Color shoes = {22, 22, 28, 255};
  for (auto& c : inv_.clothes) {
    if (c.find("hoodie") != std::string::npos) shirt = {36, 42, 120, 255};
    if (c.find("jacket") != std::string::npos || c.find("leather") != std::string::npos) shirt = {28, 28, 32, 255};
    if (c.find("suit") != std::string::npos) { shirt = {24, 24, 32, 255}; pants = {20, 20, 28, 255}; }
  }
  // soft contact shadow
  DrawCylinder(Vector3{pos.x, pos.y + 0.02f, pos.z}, 0.45f * scale, 0.45f * scale, 0.02f, 12, Fade(BLACK, 0.35f));
  // legs
  DrawCubeV(Vector3{pos.x - 0.12f * scale, pos.y + 0.35f * scale + walk, pos.z}, Vector3{0.14f * scale, 0.55f * scale, 0.14f * scale}, pants);
  DrawCubeV(Vector3{pos.x + 0.12f * scale, pos.y + 0.35f * scale - walk, pos.z}, Vector3{0.14f * scale, 0.55f * scale, 0.14f * scale}, pants);
  DrawCubeV(Vector3{pos.x - 0.12f * scale, pos.y + 0.06f * scale + walk, pos.z + 0.04f * scale}, Vector3{0.16f * scale, 0.08f * scale, 0.22f * scale}, shoes);
  DrawCubeV(Vector3{pos.x + 0.12f * scale, pos.y + 0.06f * scale - walk, pos.z + 0.04f * scale}, Vector3{0.16f * scale, 0.08f * scale, 0.22f * scale}, shoes);
  // torso
  DrawCubeV(Vector3{pos.x, pos.y + 0.95f * scale + breath, pos.z}, Vector3{0.42f * scale, 0.55f * scale, 0.24f * scale}, shirt);
  // arms
  DrawCubeV(Vector3{pos.x - 0.32f * scale, pos.y + 0.95f * scale - walk * 0.5f, pos.z}, Vector3{0.12f * scale, 0.45f * scale, 0.12f * scale}, shirt);
  DrawCubeV(Vector3{pos.x + 0.32f * scale, pos.y + 0.95f * scale + walk * 0.5f, pos.z}, Vector3{0.12f * scale, 0.45f * scale, 0.12f * scale}, shirt);
  DrawSphere(Vector3{pos.x - 0.32f * scale, pos.y + 0.68f * scale - walk * 0.5f, pos.z}, 0.07f * scale, skin);
  DrawSphere(Vector3{pos.x + 0.32f * scale, pos.y + 0.68f * scale + walk * 0.5f, pos.z}, 0.07f * scale, skin);
  // head
  Vector3 hp{pos.x, pos.y + 1.45f * scale + breath, pos.z};
  DrawSphere(hp, 0.18f * scale, skin);
  // hair
  DrawSphere(Vector3{hp.x, hp.y + 0.08f * scale, hp.z}, 0.17f * scale, hairColor_);
  // eyes
  DrawSphere(Vector3{hp.x - 0.06f * scale, hp.y + 0.02f * scale, hp.z + 0.14f * scale}, 0.03f * scale, WHITE);
  DrawSphere(Vector3{hp.x + 0.06f * scale, hp.y + 0.02f * scale, hp.z + 0.14f * scale}, 0.03f * scale, WHITE);
  DrawSphere(Vector3{hp.x - 0.06f * scale, hp.y + 0.02f * scale, hp.z + 0.16f * scale}, 0.015f * scale, BLACK);
  DrawSphere(Vector3{hp.x + 0.06f * scale, hp.y + 0.02f * scale, hp.z + 0.16f * scale}, 0.015f * scale, BLACK);
  // rim-ish outline using slightly larger dark sphere behind - skip in 3D, shader handles
}

void Character::DrawCar(float x, float y) {
  if (activeCar_.empty()) return;
  Color body = {carR_, carG_, carB_, 255};
  Color bodyDark = Shade(body, 0.65f);
  Color bodyLight = Shade(body, 1.2f);
  float side = fabsf(sinf(facingAngle_));
  int w = (int)(78 + side * 24);
  int h = 30;
  // shadow
  DrawEllipse((int)x, (int)(y + 22), w / 2 + 6, 10, Fade(BLACK, 0.4f));
  // body 3d
  DrawRectangleRounded({x - w / 2.f, y - h / 2.f, (float)w, (float)h}, 0.35f, 8, body);
  DrawRectangleRounded({x - w / 2.f, y - h / 2.f, w * 0.25f, (float)h}, 0.35f, 6, bodyDark);
  DrawRectangleRounded({x + w * 0.15f, y - h / 2.f, w * 0.2f, (float)h}, 0.35f, 6, Fade(bodyLight, 0.4f));
  // cabin
  DrawRectangleRounded({x - w / 2.f + 14, y - h / 2.f - 14, (float)w - 28, 16}, 0.25f, 6, bodyDark);
  DrawRectangleRounded({x - w / 2.f + 18, y - h / 2.f - 12, (float)w - 36, 10}, 0.2f, 4, Color{30, 40, 55, 200});
  // wheels 3d
  DrawCircle((int)(x - w / 3), (int)(y + h / 2), 9, Color{25, 25, 28, 255});
  DrawCircle((int)(x - w / 3), (int)(y + h / 2), 5, Color{60, 60, 65, 255});
  DrawCircle((int)(x + w / 3), (int)(y + h / 2), 9, Color{25, 25, 28, 255});
  DrawCircle((int)(x + w / 3), (int)(y + h / 2), 5, Color{60, 60, 65, 255});
  if (neonCar_)
    DrawEllipse((int)x, (int)(y + h / 2 + 6), w / 2, 7, Fade(Color{0, 220, 255, 255}, 0.5f));
  if (turboLvl_ > 0)
    DrawTextRu("TURBO", (int)(x - 22), (int)(y - 42), 14, Color{244, 196, 48, 255});
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
