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
  const float S = 1.8f;
  float breath = sinf(animTime_ * 2.2f) * 3.5f;
  float ang = facingAngle_;
  float turn = cosf(ang);
  float side = sinf(ang);
  float bodyW = (20.f + fabsf(turn) * 8.f) * S;
  float depth = side * 8.f * S;

  Color skin = female_ ? Color{225, 180, 155, 255} : Color{205, 165, 135, 255};
  Color skinDark = Shade(skin, 0.78f);
  Color shirt = {45, 45, 60, 255};
  Color pants = {28, 28, 40, 255};
  Color shoes = {18, 18, 22, 255};
  for (auto& c : inv_.clothes) {
    if (c.find("hoodie") != std::string::npos || c.find("Худи") != std::string::npos || c.find("hudi") != std::string::npos)
      shirt = {35, 40, 110, 255};
    if (c.find("jacket") != std::string::npos || c.find("kurt") != std::string::npos)
      shirt = {25, 25, 30, 255};
    if (c.find("suit") != std::string::npos || c.find("kostyum") != std::string::npos)
      shirt = {20, 20, 28, 255};
    if (c.find("sport") != std::string::npos)
      shirt = {180, 40, 40, 255};
  }

  // soft ground shadow
  DrawEllipse((int)x, (int)(y + 52 * S), (int)(28 * S), (int)(10 * S), Fade(BLACK, 0.35f));

  float legSwing = sinf(animTime_ * 8.f) * 6.f * S * (fabsf(side) > 0.2f ? 1.f : 0.3f);
  // legs
  DrawRectangleRounded(Rectangle{x - 12 * S + legSwing, y + 10 * S, 10 * S, 28 * S}, 0.3f, 4, pants);
  DrawRectangleRounded(Rectangle{x + 2 * S - legSwing, y + 10 * S, 10 * S, 28 * S}, 0.3f, 4, pants);
  DrawRectangleRounded(Rectangle{x - 13 * S + legSwing, y + 36 * S, 12 * S, 8 * S}, 0.2f, 4, shoes);
  DrawRectangleRounded(Rectangle{x + 1 * S - legSwing, y + 36 * S, 12 * S, 8 * S}, 0.2f, 4, shoes);

  // torso with thickness
  DrawRectangleRounded(Rectangle{x - bodyW * 0.5f + depth * 0.15f, y - 28 * S + breath, bodyW, 40 * S}, 0.2f, 6, shirt);
  DrawRectangleRounded(Rectangle{x - bodyW * 0.5f - 2 * S, y - 28 * S + breath, 5 * S, 40 * S}, 0.2f, 4, Shade(shirt, 0.7f));

  // arms
  float armSwing = -legSwing * 0.8f;
  DrawRectangleRounded(Rectangle{x - bodyW * 0.55f - 6 * S, y - 24 * S + breath + armSwing, 8 * S, 26 * S}, 0.4f, 4, skin);
  DrawRectangleRounded(Rectangle{x + bodyW * 0.35f, y - 24 * S + breath - armSwing, 8 * S, 26 * S}, 0.4f, 4, skin);

  // head
  float hx = x + depth * 0.1f, hy = y - 48 * S + breath;
  DrawCircle((int)hx, (int)hy, 14 * S, skin);
  DrawCircle((int)(hx - 3 * S), (int)(hy + 2 * S), 12 * S, Fade(skinDark, 0.35f));

  // eyes
  float blink = (sinf(animTime_ * 1.7f) > 0.92f) ? 1.f : 3.5f * S;
  DrawRectangleRounded(Rectangle{hx - 7 * S, hy - 2 * S, 5 * S, blink}, 0.4f, 2, WHITE);
  DrawRectangleRounded(Rectangle{hx + 2 * S, hy - 2 * S, 5 * S, blink}, 0.4f, 2, WHITE);
  DrawCircle((int)(hx - 4.5f * S), (int)hy, 1.8f * S, BLACK);
  DrawCircle((int)(hx + 4.5f * S), (int)hy, 1.8f * S, BLACK);

  // hair volume
  Color hair = hairColor_;
  DrawCircle((int)hx, (int)(hy - 6 * S), 13 * S, hair);
  if (hairStyle_ % 4 == 1) {
    DrawCircle((int)(hx - 10 * S), (int)hy, 6 * S, hair);
    DrawCircle((int)(hx + 10 * S), (int)hy, 6 * S, hair);
  } else if (hairStyle_ % 4 == 2) {
    DrawRectangleRounded(Rectangle{hx - 5 * S, hy - 18 * S, 10 * S, 10 * S}, 0.3f, 4, hair);
  } else if (female_ || hairStyle_ % 4 == 3) {
    DrawCircle((int)(hx - 9 * S), (int)(hy + 8 * S), 7 * S, hair);
    DrawCircle((int)(hx + 9 * S), (int)(hy + 8 * S), 7 * S, hair);
  }

  // accessories from clothes
  for (auto& c : inv_.clothes) {
    if (c.find("glasses") != std::string::npos || c.find("ochki") != std::string::npos) {
      DrawRectangleRounded(Rectangle{hx - 10 * S, hy - 3 * S, 9 * S, 5 * S}, 0.2f, 2, Color{20, 20, 25, 220});
      DrawRectangleRounded(Rectangle{hx + 1 * S, hy - 3 * S, 9 * S, 5 * S}, 0.2f, 2, Color{20, 20, 25, 220});
    }
    if (c.find("chain") != std::string::npos || c.find("tsep") != std::string::npos) {
      DrawCircleLines((int)x, (int)(y - 12 * S + breath), (int)(8 * S), Color{244, 196, 48, 255});
    }
    if (c.find("bandana") != std::string::npos) {
      DrawRectangleRounded(Rectangle{hx - 12 * S, hy - 4 * S, 24 * S, 6 * S}, 0.2f, 2, Color{180, 30, 30, 255});
    }
  }

  // weapon in hand
  if (!inv_.weapons.empty()) {
    DrawRectangleRounded(Rectangle{x + bodyW * 0.4f, y - 10 * S, 6 * S, 22 * S}, 0.2f, 2, Color{40, 40, 48, 255});
  }

  // rim light outline
  DrawCircleLines((int)hx, (int)hy, (int)(15 * S), Fade(Color{0, 200, 255, 255}, 0.25f));
  DrawCircleLines((int)x, (int)(y - 8 * S), (int)(22 * S), Fade(Color{220, 40, 80, 255}, 0.12f));
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
