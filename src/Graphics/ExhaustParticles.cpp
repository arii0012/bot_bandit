#include "ExhaustParticles.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

void ExhaustParticles::Reset() {
  for (auto& p : pool_) p.alive = false;
  cursor_ = 0;
}

void ExhaustParticles::Emit(float x, float y, float angle, float intensity, bool nitro) {
  int count = 2 + static_cast<int>(intensity * 4.f) + (nitro ? 3 : 0);
  float back = angle + 3.14159f;
  for (int i = 0; i < count; ++i) {
    auto& p = pool_[cursor_ % MAX];
    cursor_++;
    float spread = ((rand() % 100) / 100.f - 0.5f) * 0.7f;
    float a = back + spread;
    float spd = 50.f + intensity * 60.f + static_cast<float>(rand() % 40);
    p.x = x - cosf(angle) * 28.f;
    p.y = y - sinf(angle) * 28.f;
    p.vx = cosf(a) * spd;
    p.vy = sinf(a) * spd - 20.f;
    p.maxLife = 0.4f + (rand() % 50) / 100.f;
    p.life = p.maxLife;
    p.size = 3.5f + intensity * 5.f + (nitro ? 4.f : 0.f);
    if (nitro) {
      p.r = 255;
      p.g = static_cast<unsigned char>(100 + rand() % 100);
      p.b = 20;
    } else {
      unsigned char g = static_cast<unsigned char>(50 + rand() % 60);
      p.r = p.g = p.b = g;
    }
    p.a = 200;
    p.alive = true;
  }
}

void ExhaustParticles::EmitSparks(float x, float y, int n) {
  for (int i = 0; i < n; ++i) {
    auto& p = pool_[cursor_ % MAX];
    cursor_++;
    float a = (rand() % 628) / 100.f;
    float spd = 80.f + rand() % 80;
    p.x = x; p.y = y;
    p.vx = cosf(a) * spd;
    p.vy = sinf(a) * spd - 40.f;
    p.maxLife = 0.25f + (rand() % 30) / 100.f;
    p.life = p.maxLife;
    p.size = 2.f + (rand() % 20) / 10.f;
    p.r = 255; p.g = static_cast<unsigned char>(180 + rand() % 60); p.b = 40;
    p.a = 255;
    p.alive = true;
  }
}

void ExhaustParticles::Update(float dt) {
  if (dt > 0.1f) dt = 0.1f;
  for (auto& p : pool_) {
    if (!p.alive) continue;
    p.life -= dt;
    if (p.life <= 0.f) { p.alive = false; continue; }
    p.x += p.vx * dt;
    p.y += p.vy * dt;
    p.vx *= (1.f - 1.6f * dt);
    p.vy *= (1.f - 1.3f * dt);
    p.vy -= 25.f * dt;
    p.size += 10.f * dt;
  }
}

void ExhaustParticles::Draw() {
  for (auto& p : pool_) {
    if (!p.alive) continue;
    float t = p.life / p.maxLife;
    unsigned char a = static_cast<unsigned char>(p.a * t);
    DrawCircle(static_cast<int>(p.x), static_cast<int>(p.y), p.size, Color{p.r, p.g, p.b, a});
  }
}
