#include "Character.hpp"
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
  float breath = sinf(animTime_ * 2.2f) * 2.2f;
  float ang = facingAngle_;
  float turn = cosf(ang);
  float side = sinf(ang);
  float bodyW = 20.f + fabsf(turn) * 8.f;
  float depth = side * 8.f;

  Color skin = female_ ? Color{225, 180, 155, 255} : Color{205, 165, 135, 255};
  Color skinDark = Shade(skin, 0.78f);
  Color shirt = {45, 45, 60, 255};
  Color pants = {28, 28, 40, 255};
  Color shoes = {18, 18, 22, 255};
  for (auto& c : inv_.clothes) {
    if (c.find("hoodie") != std::string::npos || c.find("Худи") != std::string::npos)
      shirt = {35, 40, 110, 255};
    if (c.find("suit") != std::string::npos || c.find("костюм") != std::string::npos)
      shirt = {35, 35, 42, 255};
    if (c.find("leather") != std::string::npos || c.find("кожан") != std::string::npos)
      shirt = {55, 32, 28, 255};
  }
  Color shirtDark = Shade(shirt, 0.72f);
  Color shirtLight = Shade(shirt, 1.15f);

  // ground shadow (ellipse 3d)
  DrawEllipse((int)x, (int)(y + 56), (int)(26 + fabsf(side) * 4), 10, Fade(BLACK, 0.4f));

  // === BACK ARM (depth layer) ===
  float armX = x - 14.f + depth * 0.8f;
  DrawRectangleRounded({armX, y + 6 + breath, 8, 22}, 0.45f, 6, skinDark);
  DrawCircle((int)(armX + 4), (int)(y + 28 + breath), 5, skinDark);

  // === LEGS with 3D side shading ===
  DrawRectangleRounded({x - 12, y + 32, 10, 24}, 0.25f, 5, Shade(pants, 0.85f));
  DrawRectangleRounded({x + 2, y + 32, 10, 24}, 0.25f, 5, pants);
  // shoes perspective
  DrawRectangleRounded({x - 13, y + 52, 12, 7}, 0.3f, 4, shoes);
  DrawRectangleRounded({x + 1, y + 52, 12, 7}, 0.3f, 4, shoes);
  DrawRectangle({(int)(x - 13), (int)(y + 56), 12, 3}, Shade(shoes, 0.6f));
  DrawRectangle({(int)(x + 1), (int)(y + 56), 12, 3}, Shade(shoes, 0.6f));

  // === TORSO 3D: left shade + right light ===
  float ty = y + breath;
  DrawRectangleRounded({x - bodyW / 2, ty, bodyW, 34}, 0.2f, 8, shirt);
  // left edge dark
  DrawRectangleRounded({x - bodyW / 2, ty, bodyW * 0.28f, 34}, 0.2f, 6, shirtDark);
  // right edge light
  DrawRectangleRounded({x + bodyW * 0.15f, ty, bodyW * 0.28f, 34}, 0.2f, 6, Fade(shirtLight, 0.5f));
  // collar
  DrawRectangleRounded({x - 8, ty - 2, 16, 6}, 0.4f, 4, Shade(shirt, 0.9f));

  // === FRONT ARM ===
  float arm2 = x + 10.f - depth * 0.8f;
  DrawRectangleRounded({arm2, y + 6 + breath, 8, 22}, 0.45f, 6, skin);
  DrawCircle((int)(arm2 + 4), (int)(y + 28 + breath), 5, skin);

  // === HEAD with sphere-like shading ===
  float hx = x, hy = y - 16 + breath * 0.5f;
  DrawCircle((int)hx, (int)hy, 16, skin);
  DrawCircle((int)(hx - 5), (int)(hy + 2), 14, Fade(skinDark, 0.35f)); // soft shadow
  DrawCircle((int)(hx + 4), (int)(hy - 3), 7, Fade(WHITE, 0.12f)); // highlight

  // hair volume by style
  float hyy = hy - 10;
  if (hairStyle_ % 4 == 0) {
    DrawCircle((int)hx, (int)hyy, 14, hairColor_);
    DrawCircle((int)(hx - 8), (int)(hyy + 4), 8, Shade(hairColor_, 0.85f));
  } else if (hairStyle_ % 4 == 1) {
    DrawCircle((int)hx, (int)hyy, 13, hairColor_);
    DrawRectangleRounded({hx - 16, hyy - 2, 10, 20}, 0.4f, 4, hairColor_);
    DrawRectangleRounded({hx + 6, hyy - 2, 10, 20}, 0.4f, 4, hairColor_);
  } else if (hairStyle_ % 4 == 2) {
    DrawCircle((int)hx, (int)(hyy - 2), 15, hairColor_);
    DrawRectangle((int)(hx - 5), (int)(hyy - 16), 10, 10, hairColor_);
  } else {
    DrawCircle((int)hx, (int)hyy, 12, hairColor_);
    if (female_) {
      DrawRectangleRounded({hx - 18, hyy, 8, 22}, 0.5f, 4, hairColor_);
      DrawRectangleRounded({hx + 10, hyy, 8, 22}, 0.5f, 4, hairColor_);
    }
  }

  // eyes with depth
  Vector2 m = GetMousePosition();
  float dx = std::clamp((m.x - x) * 0.012f, -2.5f, 2.5f);
  float eyeY = hy - 2;
  if (fabsf(turn) < 0.35f) {
    DrawCircle((int)(hx - 5 + dx), (int)eyeY, 3, WHITE);
    DrawCircle((int)(hx + 5 + dx), (int)eyeY, 3, WHITE);
    DrawCircle((int)(hx - 5 + dx), (int)eyeY, 2, BLACK);
    DrawCircle((int)(hx + 5 + dx), (int)eyeY, 2, BLACK);
  } else if (turn > 0) {
    DrawCircle((int)(hx + 6 + dx), (int)eyeY, 3, WHITE);
    DrawCircle((int)(hx + 6 + dx), (int)eyeY, 2, BLACK);
  } else {
    DrawCircle((int)(hx - 6 + dx), (int)eyeY, 3, WHITE);
    DrawCircle((int)(hx - 6 + dx), (int)eyeY, 2, BLACK);
  }

  // accessories
  for (auto& c : inv_.clothes) {
    if (c.find("chain") != std::string::npos || c.find("цеп") != std::string::npos || c.find("gold") != std::string::npos) {
      DrawCircleLines((int)x, (int)(ty + 10), 6, Color{244, 196, 48, 255});
      DrawCircleLines((int)x, (int)(ty + 10), 5, Color{244, 196, 48, 180});
    }
  }
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
    DrawText("TURBO", (int)(x - 22), (int)(y - 42), 14, Color{244, 196, 48, 255});
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
