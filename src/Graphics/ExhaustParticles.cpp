#include "ExhaustParticles.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

void ExhaustParticles::Reset() {
  for (auto& p : pool_) p.alive = false;
  cursor_ = 0;
}

void ExhaustParticles::Emit(float x, float y, float angle, float intensity, bool nitro) {
  int count = 1 + (int)(intensity * 3.f) + (nitro ? 2 : 0);
  float back = angle + 3.14159f; // behind car
  for (int i = 0; i < count; ++i) {
    auto& p = pool_[cursor_ % MAX];
    cursor_++;
    float spread = ((rand() % 100) / 100.f - 0.5f) * 0.6f;
    float a = back + spread;
    float spd = 40.f + intensity * 50.f + (rand() % 30);
    p.x = x - cosf(angle) * 26.f;
    p.y = y - sinf(angle) * 26.f;
    p.vx = cosf(a) * spd;
    p.vy = sinf(a) * spd - 15.f;
    p.maxLife = 0.35f + (rand() % 40) / 100.f;
    p.life = p.maxLife;
    p.size = 3.f + intensity * 4.f + (nitro ? 3.f : 0.f);
    if (nitro) {
      p.r = 255; p.g = (unsigned char)(120 + rand() % 80); p.b = 20;
    } else {
      p.r = p.g = p.b = (unsigned char)(60 + rand() % 50);
    }
    p.a = 180;
    p.alive = true;
  }
}

void ExhaustParticles::Update(float dt) {
  for (auto& p : pool_) {
    if (!p.alive) continue;
    p.life -= dt;
    if (p.life <= 0) { p.alive = false; continue; }
    p.x += p.vx * dt;
    p.y += p.vy * dt;
    p.vx *= (1.f - 1.5f * dt);
    p.vy *= (1.f - 1.2f * dt);
    p.vy -= 20.f * dt; // rise
    p.size += 8.f * dt;
  }
}

void ExhaustParticles::Draw() {
  for (auto& p : pool_) {
    if (!p.alive) continue;
    float t = p.life / p.maxLife;
    unsigned char a = (unsigned char)(p.a * t);
    DrawCircle((int)p.x, (int)p.y, p.size, Color{p.r, p.g, p.b, a});
  }
}
