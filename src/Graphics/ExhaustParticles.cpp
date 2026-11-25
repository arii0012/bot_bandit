#include "ExhaustParticles.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

ExhaustParticles::P& ExhaustParticles::Next() {
  P& p = pool_[cursor_ % MAX];
  cursor_++;
  return p;
}

void ExhaustParticles::Reset() {
  for (auto& p : pool_) p.alive = false;
  cursor_ = 0;
}

void ExhaustParticles::Emit(float x, float y, float angle, float intensity, bool nitro) {
  int count = 2 + static_cast<int>(intensity * 4.f) + (nitro ? 3 : 0);
  float back = angle + 3.14159f;
  for (int i = 0; i < count; ++i) {
    auto& p = Next();
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
    p.angle = a;
    p.kind = nitro ? ParticleKind::Spark : ParticleKind::Exhaust;
    if (nitro) {
      p.r = 255; p.g = static_cast<unsigned char>(100 + rand() % 100); p.b = 20;
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
    auto& p = Next();
    float a = (rand() % 628) / 100.f;
    float spd = 80.f + static_cast<float>(rand() % 80);
    p.x = x; p.y = y;
    p.vx = cosf(a) * spd;
    p.vy = sinf(a) * spd - 40.f;
    p.maxLife = 0.25f + (rand() % 30) / 100.f;
    p.life = p.maxLife;
    p.size = 2.f + (rand() % 20) / 10.f;
    p.r = 255; p.g = static_cast<unsigned char>(180 + rand() % 60); p.b = 40;
    p.a = 255;
    p.kind = ParticleKind::Spark;
    p.alive = true;
  }
}

void ExhaustParticles::EmitDriftSmoke(float x, float y, float angle, float intensity) {
  int n = 3 + static_cast<int>(intensity * 5.f);
  for (int i = 0; i < n; ++i) {
    auto& p = Next();
    float a = angle + 3.14159f + ((rand() % 100) / 100.f - 0.5f);
    p.x = x + cosf(a) * 10.f;
    p.y = y + sinf(a) * 6.f;
    p.vx = cosf(a) * (20.f + intensity * 30.f);
    p.vy = -15.f - intensity * 20.f;
    p.maxLife = 0.6f + intensity * 0.4f;
    p.life = p.maxLife;
    p.size = 8.f + intensity * 12.f;
    p.r = p.g = p.b = static_cast<unsigned char>(80 + rand() % 40);
    p.a = 160;
    p.kind = ParticleKind::DriftSmoke;
    p.alive = true;
  }
}

void ExhaustParticles::EmitRain(int sw, int sh, float density) {
  int n = static_cast<int>(8.f * density);
  for (int i = 0; i < n; ++i) {
    auto& p = Next();
    p.x = static_cast<float>(rand() % std::max(1, sw));
    p.y = static_cast<float>(-(rand() % 40));
    p.vx = -40.f;
    p.vy = 420.f + static_cast<float>(rand() % 120);
    p.maxLife = 1.2f;
    p.life = p.maxLife;
    p.size = 1.5f + (rand() % 10) / 10.f;
    p.r = 180; p.g = 200; p.b = 255; p.a = 160;
    p.kind = ParticleKind::Rain;
    p.alive = true;
  }
}

void ExhaustParticles::EmitLeaf(float x, float y) {
  auto& p = Next();
  p.x = x; p.y = y;
  p.vx = -20.f + (rand() % 40);
  p.vy = 20.f + (rand() % 30);
  p.maxLife = 2.5f;
  p.life = p.maxLife;
  p.size = 4.f + (rand() % 4);
  p.r = static_cast<unsigned char>(180 + rand() % 60);
  p.g = static_cast<unsigned char>(80 + rand() % 80);
  p.b = 30; p.a = 220;
  p.kind = ParticleKind::Leaf;
  p.angle = (rand() % 628) / 100.f;
  p.alive = true;
}

void ExhaustParticles::EmitMuzzle(float x, float y, float angle) {
  for (int i = 0; i < 6; ++i) {
    auto& p = Next();
    float a = angle + ((rand() % 100) / 100.f - 0.5f) * 0.4f;
    p.x = x; p.y = y;
    p.vx = cosf(a) * (120.f + rand() % 80);
    p.vy = sinf(a) * (120.f + rand() % 80);
    p.maxLife = 0.12f + (rand() % 10) / 100.f;
    p.life = p.maxLife;
    p.size = 3.f + (rand() % 4);
    p.r = 255; p.g = 220; p.b = 80; p.a = 255;
    p.kind = ParticleKind::Muzzle;
    p.alive = true;
  }
  // casing
  auto& c = Next();
  c.x = x; c.y = y;
  c.vx = cosf(angle + 1.2f) * 60.f;
  c.vy = -80.f;
  c.maxLife = 0.5f;
  c.life = c.maxLife;
  c.size = 2.f;
  c.r = 220; c.g = 180; c.b = 60; c.a = 255;
  c.kind = ParticleKind::Casing;
  c.alive = true;
}

void ExhaustParticles::EmitConfetti(float x, float y, int n) {
  for (int i = 0; i < n; ++i) {
    auto& p = Next();
    float a = (rand() % 628) / 100.f;
    p.x = x; p.y = y;
    p.vx = cosf(a) * (40.f + rand() % 80);
    p.vy = sinf(a) * (40.f + rand() % 80) - 50.f;
    p.maxLife = 1.2f + (rand() % 50) / 100.f;
    p.life = p.maxLife;
    p.size = 3.f + (rand() % 5);
    p.r = static_cast<unsigned char>(GetRandomValue(40, 255));
    p.g = static_cast<unsigned char>(GetRandomValue(40, 255));
    p.b = static_cast<unsigned char>(GetRandomValue(40, 255));
    p.a = 255;
    p.kind = ParticleKind::Confetti;
    p.alive = true;
  }
}

void ExhaustParticles::EmitNeonSpark(float x, float y) {
  auto& p = Next();
  p.x = x; p.y = y;
  p.vx = (rand() % 40) - 20.f;
  p.vy = -30.f - (rand() % 40);
  p.maxLife = 0.4f;
  p.life = p.maxLife;
  p.size = 2.f;
  p.r = 0; p.g = 220; p.b = 255; p.a = 220;
  p.kind = ParticleKind::NeonSpark;
  p.alive = true;
}

void ExhaustParticles::Update(float dt) {
  if (dt > 0.1f) dt = 0.1f;
  for (auto& p : pool_) {
    if (!p.alive) continue;
    p.life -= dt;
    if (p.life <= 0.f) {
      if (p.kind == ParticleKind::Rain && p.y > 0.f) {
        // ripple
        p.kind = ParticleKind::RainRipple;
        p.life = 0.35f;
        p.maxLife = 0.35f;
        p.vx = 0; p.vy = 0;
        p.size = 4.f;
        p.a = 120;
        continue;
      }
      p.alive = false;
      continue;
    }
    p.x += p.vx * dt;
    p.y += p.vy * dt;
    if (p.kind == ParticleKind::DriftSmoke || p.kind == ParticleKind::Exhaust) {
      p.vx *= (1.f - 1.2f * dt);
      p.vy *= (1.f - 1.0f * dt);
      p.vy -= 18.f * dt;
      p.size += 12.f * dt;
    } else if (p.kind == ParticleKind::Leaf) {
      p.angle += dt * 3.f;
      p.vx += sinf(p.angle) * 10.f * dt;
      p.vy += 30.f * dt;
    } else if (p.kind == ParticleKind::Confetti) {
      p.vy += 90.f * dt;
      p.angle += dt * 6.f;
    } else if (p.kind == ParticleKind::Casing) {
      p.vy += 220.f * dt;
    } else if (p.kind == ParticleKind::RainRipple) {
      p.size += 40.f * dt;
    } else {
      p.vx *= (1.f - 1.4f * dt);
      p.vy *= (1.f - 1.2f * dt);
    }
  }
}

void ExhaustParticles::EmitBills(float x, float y, int n) {
  for (int i = 0; i < n; ++i) {
    auto& p = Next();
    float a = (rand() % 628) / 100.f;
    p.x = x; p.y = y;
    p.vx = cosf(a) * (30.f + rand() % 50);
    p.vy = sinf(a) * (30.f + rand() % 40) - 40.f;
    p.maxLife = 1.0f + (rand() % 40) / 100.f;
    p.life = p.maxLife;
    p.size = 4.f + (rand() % 3);
    p.r = 60; p.g = 160; p.b = 80; p.a = 230;
    p.kind = ParticleKind::Confetti;
    p.alive = true;
  }
}

void ExhaustParticles::EmitHeatHaze(float x, float y) {
  for (int i = 0; i < 4; ++i) {
    auto& p = Next();
    p.x = x + (rand() % 40) - 20.f;
    p.y = y;
    p.vx = (rand() % 20) - 10.f;
    p.vy = -40.f - (rand() % 30);
    p.maxLife = 0.5f;
    p.life = p.maxLife;
    p.size = 10.f;
    p.r = 255; p.g = 180; p.b = 80; p.a = 60;
    p.kind = ParticleKind::Dust;
    p.alive = true;
  }
}

void ExhaustParticles::EmitGlitch(float x, float y, float w, float h) {
  for (int i = 0; i < 8; ++i) {
    auto& p = Next();
    p.x = x + (rand() % (int)std::max(1.f, w));
    p.y = y + (rand() % (int)std::max(1.f, h));
    p.vx = (rand() % 200) - 100.f;
    p.vy = (rand() % 40) - 20.f;
    p.maxLife = 0.2f;
    p.life = p.maxLife;
    p.size = 2.f + (rand() % 6);
    p.r = (unsigned char)(rand() % 255);
    p.g = 40; p.b = 255; p.a = 200;
    p.kind = ParticleKind::NeonSpark;
    p.alive = true;
  }
}

void ExhaustParticles::DrawGodRays(float originX, float originY, int sw, int sh, float intensity) {
  if (intensity <= 0.01f) return;
  for (int i = 0; i < 7; ++i) {
    float a = -1.2f + i * 0.35f;
    Vector2 end{originX + cosf(a) * (float)sh * 0.9f, originY + sinf(a) * (float)sh * 0.9f};
    DrawLineEx(Vector2{originX, originY}, end, 18.f, Fade(Color{255, 230, 160, 255}, 0.04f * intensity));
  }
  (void)sw;
}

void ExhaustParticles::Draw() {

  for (auto& p : pool_) {
    if (!p.alive) continue;
    float t = p.life / p.maxLife;
    unsigned char a = static_cast<unsigned char>(p.a * t);
    Color c{p.r, p.g, p.b, a};
    if (p.kind == ParticleKind::Rain) {
      DrawLineEx(Vector2{p.x, p.y}, Vector2{p.x + p.vx * 0.02f, p.y + 10.f}, 1.5f, c);
    } else if (p.kind == ParticleKind::RainRipple) {
      DrawCircleLines(static_cast<int>(p.x), static_cast<int>(p.y), p.size, c);
    } else if (p.kind == ParticleKind::Leaf || p.kind == ParticleKind::Confetti) {
      DrawRectanglePro(Rectangle{p.x, p.y, p.size, p.size * 0.6f}, Vector2{p.size * 0.5f, p.size * 0.3f},
                       p.angle * RAD2DEG, c);
    } else {
      DrawCircle(static_cast<int>(p.x), static_cast<int>(p.y), p.size, c);
    }
  }
}
