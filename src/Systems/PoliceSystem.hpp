#pragma once
#include "Character/Character.hpp"
#include "Systems/CrimeAndRacketSystem.hpp"
#include "Systems/ReputationScales.hpp"
#include "Systems/ArrestAndJailSystem.hpp"
#include "Core/FontUI.hpp"
#include "raylib.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <string>

struct PatrolUnit {
  float x, y;
  float vx, vy;
  float vision;
  float alert;
};

class PoliceSystem {
public:
  void Init(int sw, int sh) {
    units_.clear();
    for (int i = 0; i < 4; ++i) {
      PatrolUnit u;
      u.x = static_cast<float>(GetRandomValue(40, sw - 40));
      u.y = static_cast<float>(GetRandomValue(80, sh - 120));
      u.vx = (GetRandomValue(0, 1) ? 1.f : -1.f) * 40.f;
      u.vy = (GetRandomValue(0, 1) ? 1.f : -1.f) * 25.f;
      u.vision = 90.f;
      u.alert = 0.f;
      units_.push_back(u);
    }
  }

  void Update(float dt, Character& p, int sw, int sh, CrimeAndRacketSystem& crime, ArrestAndJailSystem& jail) {
    if (units_.empty()) Init(sw, sh);
    float px = sw * 0.5f, py = sh * 0.55f;
    float aggro = 1.f + p.Wanted() * 0.35f + (crime.IsHot() ? 0.5f : 0.f);

    for (auto& u : units_) {
      u.x += u.vx * dt;
      u.y += u.vy * dt;
      if (u.x < 20 || u.x > sw - 20) u.vx = -u.vx;
      if (u.y < 40 || u.y > sh - 100) u.vy = -u.vy;

      float dx = px - u.x, dy = py - u.y;
      float dist = sqrtf(dx * dx + dy * dy);
      if (dist < u.vision * aggro && p.Wanted() > 0) {
        u.alert = std::min(1.f, u.alert + dt * 0.4f * aggro);
        // chase
        u.vx = (dx / (dist + 0.1f)) * 70.f;
        u.vy = (dy / (dist + 0.1f)) * 70.f;
        if (dist < 40.f && u.alert > 0.8f && p.Wanted() >= 2) {
          CrimeArticle art = CrimeArticle::ResistArrest;
          if (p.Wanted() >= 4) art = CrimeArticle::Shooting;
          jail.Arrest(p, art, 25.f * p.Wanted());
          crime.AddConviction();
          p.SetWanted(0);
          u.alert = 0.f;
        }
      } else {
        u.alert = std::max(0.f, u.alert - dt * 0.2f);
      }
    }
  }

  void Draw() {
    for (auto& u : units_) {
      Color c = u.alert > 0.3f ? Color{40, 80, 200, 255} : Color{30, 50, 120, 255};
      DrawRectangleRec(Rectangle{u.x - 14, u.y - 8, 28, 16}, c);
      DrawCircle((int)(u.x - 8), (int)(u.y + 8), 5, BLACK);
      DrawCircle((int)(u.x + 8), (int)(u.y + 8), 5, BLACK);
      if (u.alert > 0.1f) {
        DrawCircleLines((int)u.x, (int)u.y, (int)(u.vision * (0.5f + u.alert * 0.5f)),
                        Fade(Color{255, 60, 60, 255}, u.alert * 0.35f));
      }
    }
  }

private:
  std::vector<PatrolUnit> units_;
};
