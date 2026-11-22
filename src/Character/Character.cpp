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
  animTime_ += 0.016f;
  float breath = sinf(animTime_ * 2.2f) * 2.f;
  float turn = cosf(facingAngle_);
  float depth = sinf(facingAngle_); // side layer offset

  Color skin = {210, 170, 140, 255};
  Color shirt = {40, 40, 55, 255};
  Color pants = {25, 25, 35, 255};
  Color shoes = {15, 15, 18, 255};
  // equipped clothes tint
  if (!Inv().clothes.empty()) {
    shirt = {60, 50, 80, 255};
  }
  for (auto& c : Inv().clothes) {
    if (c.find("hoodie") != std::string::npos || c.find("Худи") != std::string::npos)
      shirt = {30, 30, 90, 255};
    if (c.find("suit") != std::string::npos || c.find("костюм") != std::string::npos)
      shirt = {40, 40, 45, 255};
    if (c.find("leather") != std::string::npos || c.find("кожан") != std::string::npos)
      shirt = {50, 30, 25, 255};
  }

  // shadow (ground layer)
  DrawEllipse((int)x, (int)(y + 52), 22, 8, Fade(BLACK, 0.35f));

  // far arm (behind, 2.5D)
  float armOff = depth * 6.f;
  DrawRectangleRounded({x - 16 + armOff, y + 4 + breath, 7, 20}, 0.4f, 4, skin);

  // legs with limp offset if damaged - visual only slight
  DrawRectangleRounded({x - 10, y + 28, 8, 22}, 0.3f, 4, pants);
  DrawRectangleRounded({x + 2, y + 28, 8, 22}, 0.3f, 4, pants);
  DrawRectangleRounded({x - 11, y + 48, 10, 6}, 0.2f, 2, shoes);
  DrawRectangleRounded({x + 1, y + 48, 10, 6}, 0.2f, 2, shoes);

  // torso layer
  float bodyW = 18 + fabsf(turn) * 6;
  DrawRectangleRounded({x - bodyW/2, y + breath, bodyW, 30}, 0.25f, 6, shirt);

  // near arm (front)
  DrawRectangleRounded({x + 8 - armOff, y + 4 + breath, 7, 20}, 0.4f, 4, skin);

  // head
  DrawCircle((int)x, (int)(y - 14 + breath * 0.5f), 14, skin);
  // hair layer with style
  float hy = y - 22 + breath * 0.5f;
  if (hairStyle_ % 3 == 0) DrawCircle((int)x, (int)hy, 12, hairColor_);
  else if (hairStyle_ % 3 == 1) {
    DrawCircle((int)x, (int)hy, 11, hairColor_);
    DrawRectangleRounded({x - 14, hy, 8, 16}, 0.3f, 4, hairColor_); // side long
  } else {
    DrawCircle((int)x, (int)(hy - 2), 13, hairColor_);
    DrawRectangle((int)(x - 4), (int)(hy - 14), 8, 8, hairColor_); // mohawk-ish
  }

  // eyes follow
  Vector2 m = GetMousePosition();
  float dx = (m.x - x) * 0.01f;
  if (dx > 2) dx = 2; if (dx < -2) dx = -2;
  float eyeY = y - 16 + breath * 0.5f;
  if (fabsf(turn) < 0.3f) { // front-ish
    DrawCircle((int)(x - 4 + dx), (int)eyeY, 2, BLACK);
    DrawCircle((int)(x + 4 + dx), (int)eyeY, 2, BLACK);
  } else if (turn > 0) {
    DrawCircle((int)(x + 5 + dx), (int)eyeY, 2, BLACK);
  } else {
    DrawCircle((int)(x - 5 + dx), (int)eyeY, 2, BLACK);
  }

  // accessory chain if gold in clothes
  for (auto& c : Inv().clothes) {
    if (c.find("chain") != std::string::npos || c.find("gold") != std::string::npos || c.find("цеп") != std::string::npos) {
      DrawCircleLines((int)x, (int)(y + 8 + breath), 5, Color{244, 196, 48, 255});
    }
  }
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
