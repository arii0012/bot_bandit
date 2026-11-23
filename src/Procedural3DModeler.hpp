#pragma once
#include "raylib.h"
#include "rlgl.h"
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

struct CharVisual3D {
  bool female = false;
  int hairStyle = 0;
  Color hair = {30, 25, 20, 255};
  Color skin = {210, 170, 140, 255};
  Color shirt = {40, 40, 60, 255};
  Color pants = {25, 25, 35, 255};
  Color shoes = {15, 15, 18, 255};
  Color accessory = {244, 196, 48, 255};
  bool hasChain = false;
  bool hasWeapon = false;
  float animTime = 0.f;
  enum class Anim { Idle, Walk, Run, Fight, Shoot, Dance, EnterCar } anim = Anim::Idle;
};

class Procedural3DModeler {
public:
  static void DrawPerson(const CharVisual3D& v, Vector3 pos, float yawDeg, float scale = 1.f) {
    float yaw = yawDeg * DEG2RAD;
    float t = v.animTime;
    float bob = 0.f, legA = 0.f, armA = 0.f;
    switch (v.anim) {
      case CharVisual3D::Anim::Walk:
        bob = sinf(t * 8.f) * 0.03f; legA = sinf(t * 8.f) * 0.45f; armA = -legA; break;
      case CharVisual3D::Anim::Run:
        bob = sinf(t * 12.f) * 0.05f; legA = sinf(t * 12.f) * 0.7f; armA = -legA; break;
      case CharVisual3D::Anim::Fight:
        armA = sinf(t * 10.f) * 0.9f; break;
      case CharVisual3D::Anim::Shoot:
        armA = 0.6f; break;
      case CharVisual3D::Anim::Dance:
        bob = sinf(t * 6.f) * 0.06f; armA = sinf(t * 6.f) * 0.8f; legA = cosf(t * 6.f) * 0.3f; break;
      case CharVisual3D::Anim::EnterCar:
        bob = -0.1f; break;
      default:
        bob = sinf(t * 2.2f) * 0.015f; break;
    }

    rlPushMatrix();
    rlTranslatef(pos.x, pos.y + bob * scale, pos.z);
    rlRotatef(yawDeg, 0, 1, 0);
    rlScalef(scale, scale, scale);

    // shadow
    DrawCylinderEx({0, 0.01f, 0}, {0, 0.012f, 0}, 0.35f, 0.35f, 12, Fade(BLACK, 0.35f));

    // legs
    DrawLimb({-0.12f, 0.55f, 0}, 0.08f, 0.55f, legA, 0, v.pants);
    DrawLimb({0.12f, 0.55f, 0}, 0.08f, 0.55f, -legA, 0, v.pants);
    DrawCube({-0.12f, 0.06f, 0.06f}, 0.14f, 0.08f, 0.22f, v.shoes);
    DrawCube({0.12f, 0.06f, 0.06f}, 0.14f, 0.08f, 0.22f, v.shoes);

    // torso
    Color shirtD = Shade(v.shirt, 0.75f);
    DrawCube({0, 1.05f, 0}, 0.42f, 0.55f, 0.24f, v.shirt);
    DrawCube({-0.18f, 1.05f, 0}, 0.08f, 0.55f, 0.24f, shirtD);
    DrawCube({0, 1.32f, 0}, 0.36f, 0.08f, 0.22f, Shade(v.shirt, 0.9f));

    // arms
    DrawLimb({-0.28f, 1.25f, 0}, 0.07f, 0.45f, armA * 0.5f, 0.15f + armA, v.skin);
    DrawLimb({0.28f, 1.25f, 0}, 0.07f, 0.45f, -armA * 0.5f, -0.15f - armA, v.skin);

    // head
    DrawSphere({0, 1.55f, 0}, 0.18f, v.skin);
    DrawSphere({-0.05f, 1.52f, 0.02f}, 0.15f, Fade(Shade(v.skin, 0.8f), 0.35f));

    // eyes blink
    float blink = (sinf(t * 1.7f) > 0.92f) ? 0.02f : 0.035f;
    DrawSphere({-0.06f, 1.57f, 0.14f}, blink, WHITE);
    DrawSphere({0.06f, 1.57f, 0.14f}, blink, WHITE);
    DrawSphere({-0.06f, 1.57f, 0.16f}, blink * 0.55f, BLACK);
    DrawSphere({0.06f, 1.57f, 0.16f}, blink * 0.55f, BLACK);

    // hair volume by style (visible 360°)
    DrawHair(v.hairStyle, v.hair, v.female);

    if (v.hasChain) {
      DrawCircle3D({0, 1.28f, 0.12f}, 0.08f, {1, 0, 0}, 90, v.accessory);
    }
    if (v.hasWeapon) {
      DrawCube({0.45f, 1.1f, 0.1f}, 0.08f, 0.08f, 0.35f, {40, 40, 45, 255});
    }

    rlPopMatrix();
  }

  static void DrawCar3D(Color body, Vector3 pos, float yawDeg, float wheelRot, bool neon, bool lightsOn, float scale = 1.f) {
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(yawDeg, 0, 1, 0);
    rlScalef(scale, scale, scale);

    DrawCylinderEx({0, 0.01f, 0}, {0, 0.015f, 0}, 1.1f, 1.1f, 16, Fade(BLACK, 0.4f));

    // body
    DrawCube({0, 0.35f, 0}, 1.8f, 0.4f, 0.9f, body);
    DrawCube({-0.7f, 0.35f, 0}, 0.25f, 0.4f, 0.9f, Shade(body, 0.7f));
    DrawCube({0.55f, 0.35f, 0}, 0.3f, 0.4f, 0.9f, Fade(Shade(body, 1.15f), 0.5f));

    // cabin
    Color glass = {30, 40, 55, 180};
    DrawCube({-0.1f, 0.7f, 0}, 0.9f, 0.35f, 0.82f, Shade(body, 0.85f));
    DrawCube({-0.1f, 0.72f, 0.0f}, 0.75f, 0.28f, 0.75f, glass);

    // wheels 3D
    float wr = 0.22f;
    Vector3 wpos[4] = {{-0.55f, 0.22f, 0.42f}, {0.55f, 0.22f, 0.42f}, {-0.55f, 0.22f, -0.42f}, {0.55f, 0.22f, -0.42f}};
    for (auto& w : wpos) {
      rlPushMatrix();
      rlTranslatef(w.x, w.y, w.z);
      rlRotatef(wheelRot, 0, 0, 1);
      DrawCylinderEx({0, 0, -0.06f}, {0, 0, 0.06f}, wr, wr, 14, {20, 20, 22, 255});
      DrawCylinderEx({0, 0, -0.04f}, {0, 0, 0.04f}, wr * 0.55f, wr * 0.55f, 10, {90, 90, 95, 255});
      rlPopMatrix();
    }

    // lights
    Color head = lightsOn ? Color{255, 240, 180, 255} : Color{200, 200, 180, 100};
    Color tail = lightsOn ? Color{255, 40, 40, 255} : Color{120, 30, 30, 100};
    DrawCube({0.92f, 0.38f, 0.28f}, 0.06f, 0.1f, 0.14f, head);
    DrawCube({0.92f, 0.38f, -0.28f}, 0.06f, 0.1f, 0.14f, head);
    DrawCube({-0.92f, 0.38f, 0.28f}, 0.06f, 0.1f, 0.14f, tail);
    DrawCube({-0.92f, 0.38f, -0.28f}, 0.06f, 0.1f, 0.14f, tail);

    if (neon) {
      DrawCylinderEx({0, 0.05f, 0}, {0, 0.06f, 0}, 0.7f, 0.7f, 12, Fade(Color{0, 220, 255, 255}, 0.45f));
    }
    rlPopMatrix();
  }

  static void DrawBuilding(Vector3 pos, float w, float h, float d, int seed, bool night) {
    Color wall = {(unsigned char)(30 + seed % 20), 30, (unsigned char)(38 + seed % 15), 255};
    DrawCube(pos, w, h, d, wall);
    // roof
    DrawCube({pos.x, pos.y + h * 0.5f + 0.05f, pos.z}, w * 1.05f, 0.1f, d * 1.05f, Shade(wall, 0.6f));
    int rows = 2 + seed % 4;
    int cols = 2 + seed % 3;
    for (int r = 0; r < rows; ++r)
      for (int c = 0; c < cols; ++c) {
        float wx = pos.x - w * 0.35f + c * (w * 0.35f);
        float wy = pos.y - h * 0.35f + r * (h * 0.28f);
        float wz = pos.z + d * 0.51f;
        bool lit = night && ((seed + r * 3 + c) % 3 != 0);
        DrawCube({wx, wy, wz}, 0.18f, 0.18f, 0.04f, lit ? Color{255, 210, 120, 220} : Color{40, 45, 55, 255});
      }
  }

  static void DrawHouseInterior(Vector3 pos, float scale = 1.f) {
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlScalef(scale, scale, scale);
    DrawCube({0, 1.2f, 0}, 6, 2.4f, 5, {50, 45, 40, 255});
    DrawCube({0, 0.05f, 0}, 6, 0.1f, 5, {60, 50, 45, 255});
    // safe
    DrawCube({2.2f, 0.5f, -1.8f}, 0.6f, 1.0f, 0.5f, {30, 30, 35, 255});
    // lamp
    DrawSphere({0, 2.1f, 0}, 0.15f, {255, 230, 150, 200});
    rlPopMatrix();
  }

private:
  static Color Shade(Color c, float k) {
    auto ch = [](unsigned char v, float k) -> unsigned char {
      int x = (int)(v * k); return (unsigned char)std::clamp(x, 0, 255);
    };
    return {ch(c.r, k), ch(c.g, k), ch(c.b, k), c.a};
  }

  static void DrawLimb(Vector3 origin, float rad, float len, float pitch, float swing, Color col) {
    rlPushMatrix();
    rlTranslatef(origin.x, origin.y, origin.z);
    rlRotatef(pitch * RAD2DEG, 1, 0, 0);
    rlRotatef(swing * RAD2DEG, 0, 0, 1);
    DrawCylinderEx({0, 0, 0}, {0, -len, 0}, rad, rad * 0.85f, 8, col);
    DrawSphere({0, -len, 0}, rad * 0.9f, col);
    rlPopMatrix();
  }

  static void DrawHair(int style, Color hair, bool female) {
    DrawSphere({0, 1.62f, -0.02f}, 0.19f, hair);
    if (style % 4 == 1) {
      DrawSphere({-0.14f, 1.5f, 0}, 0.1f, hair);
      DrawSphere({0.14f, 1.5f, 0}, 0.1f, hair);
    } else if (style % 4 == 2) {
      DrawCube({0, 1.75f, 0}, 0.12f, 0.16f, 0.12f, hair);
    } else if (style % 4 == 3 || female) {
      DrawSphere({-0.12f, 1.4f, -0.05f}, 0.1f, hair);
      DrawSphere({0.12f, 1.4f, -0.05f}, 0.1f, hair);
    }
  }
};
