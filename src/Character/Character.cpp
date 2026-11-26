#include "raymath.h"
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
  Color shirt = colorTop_;
  Color pants = colorPants_;
  Color shoes = colorShoes_;
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
  LoadMeshModel();
  float breathe = sinf(animTime_ * 2.2f) * 0.02f * scale;
  Vector3 drawPos = {pos.x, pos.y + breathe, pos.z};

  // contact shadow
  DrawCylinder(Vector3{pos.x, 0.02f, pos.z}, 0.35f * scale, 0.45f * scale, 0.02f, 12, Fade(BLACK, 0.35f));

  if (meshLoaded_ && mesh_.meshCount > 0) {
    float yawDeg = facingAngle_ * RAD2DEG;
    float s = scale * (female_ ? 0.92f : 1.05f);
    Color tint = colorTop_;
    DrawModelEx(mesh_, drawPos, Vector3{0, 1, 0}, yawDeg, Vector3{s, s, s}, tint);
  } else {
    // procedural detailed mesh fallback (volume body, not flat rectangles)
    Color skin = female_ ? Color{230, 180, 160, 255} : Color{210, 168, 140, 255};
    // legs
    DrawCube(Vector3{pos.x - 0.12f * scale, 0.45f * scale, pos.z}, 0.16f * scale, 0.9f * scale, 0.16f * scale, colorPants_);
    DrawCube(Vector3{pos.x + 0.12f * scale, 0.45f * scale, pos.z}, 0.16f * scale, 0.9f * scale, 0.16f * scale, colorPants_);
    // shoes
    DrawCube(Vector3{pos.x - 0.12f * scale, 0.06f * scale, pos.z + 0.04f * scale}, 0.18f * scale, 0.1f * scale, 0.28f * scale, colorShoes_);
    DrawCube(Vector3{pos.x + 0.12f * scale, 0.06f * scale, pos.z + 0.04f * scale}, 0.18f * scale, 0.1f * scale, 0.28f * scale, colorShoes_);
    // torso
    float tw = female_ ? 0.38f : 0.48f;
    DrawCube(Vector3{pos.x, 1.15f * scale, pos.z}, tw * scale, 0.7f * scale, 0.28f * scale, colorTop_);
    // head
    DrawSphere(Vector3{pos.x, 1.65f * scale, pos.z}, 0.22f * scale, skin);
    // hair
    DrawSphere(Vector3{pos.x, 1.78f * scale, pos.z}, 0.23f * scale, hairColor_);
    // eyes
    float side = cosf(facingAngle_);
    DrawSphere(Vector3{pos.x - 0.07f * scale * side, 1.68f * scale, pos.z + 0.18f * scale}, 0.035f * scale, colorEyes_);
    DrawSphere(Vector3{pos.x + 0.07f * scale * side, 1.68f * scale, pos.z + 0.18f * scale}, 0.035f * scale, colorEyes_);
    // arms
    float armSwing = sinf(animTime_ * 6.f) * 0.25f;
    DrawCube(Vector3{pos.x - 0.32f * scale, 1.1f * scale + armSwing * 0.05f, pos.z}, 0.12f * scale, 0.55f * scale, 0.12f * scale, skin);
    DrawCube(Vector3{pos.x + 0.32f * scale, 1.1f * scale - armSwing * 0.05f, pos.z}, 0.12f * scale, 0.55f * scale, 0.12f * scale, skin);
    // rim light proxy - bright outline spheres low alpha
    DrawSphereWires(Vector3{pos.x, 1.2f * scale, pos.z}, 0.55f * scale, 8, 8, Fade(Color{0, 210, 255, 255}, 0.15f));
  }
}
