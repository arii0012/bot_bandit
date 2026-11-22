#include "DriftPhysics.hpp"
#include <cmath>
#include <algorithm>

void DriftPhysics::Reset(float x, float y, float angle) {
  s_ = {};
  s_.x = x; s_.y = y; s_.angle = angle;
  s_.nitroFuel = 3.f;
  markIdx_ = 0;
  for (auto& m : marks_) m.life = 0;
}

void DriftPhysics::SetTuning(int engine, int turbo, float traction) {
  s_.engineLvl = engine;
  s_.turboLvl = turbo;
  s_.traction = std::max(0.4f, traction);
  tires_.SetSurface(traction);
}

void DriftPhysics::AddMark(float x, float y, float a) {
  marks_[markIdx_ % MAX_MARKS] = {x, y, a, 2.5f};
  markIdx_++;
}

void DriftPhysics::Update(float dt, bool accel, bool brake, bool left, bool right, bool handbrake, bool nitroPressed) {
  const float maxSpeed = 420.f + s_.engineLvl * 35.f + s_.turboLvl * 45.f;
  const float accelForce = 180.f + s_.engineLvl * 20.f + s_.turboLvl * 30.f;
  float grip = 3.2f * s_.traction;
  float turnRate = 2.4f;

  if (nitroPressed && s_.nitroFuel > 0) {
    s_.nitro = true;
    s_.nitroFuel = std::max(0.f, s_.nitroFuel - dt);
  } else {
    s_.nitro = false;
    if (s_.nitroFuel < 3.f) s_.nitroFuel += dt * 0.15f;
  }

  if (accel) s_.speed += accelForce * dt * (s_.nitro ? 1.55f : 1.f);
  if (brake) s_.speed -= accelForce * 1.4f * dt;
  if (!accel && !brake) s_.speed *= (1.f - 0.6f * dt); // drag
  if (s_.speed > maxSpeed) s_.speed = maxSpeed;
  if (s_.speed < -maxSpeed * 0.35f) s_.speed = -maxSpeed * 0.35f;

  float steer = 0.f;
  if (left) steer -= 1.f;
  if (right) steer += 1.f;

  // handbrake initiates slip
  if (handbrake && fabsf(s_.speed) > 80.f) {
    s_.slip = std::min(1.f, s_.slip + dt * 2.5f);
    grip *= 0.35f;
    turnRate *= 1.8f;
  } else {
    s_.slip = std::max(0.f, s_.slip - dt * 1.2f * s_.traction);
  }

  // angular velocity from steer * speed
  float speedFactor = std::min(1.f, fabsf(s_.speed) / 120.f);
  s_.angularVel = steer * turnRate * speedFactor;
  // counter-steer feel when sliding
  if (s_.slip > 0.3f) s_.angularVel *= (1.f + s_.slip * 0.6f);

  s_.angle += s_.angularVel * dt;

  // velocity split into forward + lateral
  float forward = s_.speed * (1.f - s_.slip * 0.5f);
  float lateral = s_.speed * s_.slip * 0.55f * (steer >= 0 ? 1.f : -1.f);
  if (handbrake) lateral *= 1.3f;

  s_.x += (cosf(s_.angle) * forward - sinf(s_.angle) * lateral) * dt;
  s_.y += (sinf(s_.angle) * forward + cosf(s_.angle) * lateral) * dt;

  s_.drifting = s_.slip > 0.35f && fabsf(s_.speed) > 90.f;
  float thr = accel ? 1.f : (brake ? -0.8f : 0.f);
  float st = (left ? -1.f : 0.f) + (right ? 1.f : 0.f);
  auto tf = tires_.Compute(s_.speed, thr, st, handbrake ? 1.f : 0.f);
  // blend tire long force into speed slightly
  s_.speed += tf.longF * 0.00002f * 60.f * dt;
  tires_.UpdateHeat(dt, s_.slip, s_.speed);

  if (s_.drifting) {
    s_.smokeTimer += dt;
    s_.tireMarkTimer += dt;
    if (s_.tireMarkTimer > 0.04f) {
      s_.tireMarkTimer = 0;
      float bx = s_.x - cosf(s_.angle) * 12.f;
      float by = s_.y - sinf(s_.angle) * 12.f;
      AddMark(bx, by, s_.angle);
    }
  }

  // grip pulls slip down when not handbraking
  if (!handbrake && s_.slip > 0) {
    s_.speed *= (1.f - (1.f - grip / 4.f) * dt * 0.15f);
  }
}

void DriftPhysics::Draw(float /*camX*/, float /*camY*/, Color body, bool neon) {
  // tire marks
  for (auto& m : marks_) {
    if (m.life <= 0) continue;
    m.life -= GetFrameTime();
    float a = std::min(1.f, m.life / 2.5f);
    Vector2 p1 = {m.x - cosf(m.a) * 8.f, m.y - sinf(m.a) * 8.f};
    Vector2 p2 = {m.x + cosf(m.a) * 8.f, m.y + sinf(m.a) * 8.f};
    DrawLineEx(p1, p2, 4.f, Fade(Color{20, 20, 20, 255}, a * 0.7f));
  }

  float x = s_.x, y = s_.y, ang = s_.angle;
  // car body as rotated rect approximation
  Vector2 corners[4];
  float hw = 28, hh = 12;
  float ca = cosf(ang), sa = sinf(ang);
  corners[0] = {x + ca * hw - sa * hh, y + sa * hw + ca * hh};
  corners[1] = {x - ca * hw - sa * hh, y - sa * hw + ca * hh};
  corners[2] = {x - ca * hw + sa * hh, y - sa * hw - ca * hh};
  corners[3] = {x + ca * hw + sa * hh, y + sa * hw - ca * hh};
  DrawTriangle(corners[0], corners[1], corners[2], body);
  DrawTriangle(corners[0], corners[2], corners[3], body);

  if (neon) {
    DrawCircleV({x, y + 10}, 18, Fade(Color{0, 220, 255, 255}, 0.35f));
  }
  if (s_.nitro) {
    DrawCircleV({x - ca * 30, y - sa * 30}, 10, Fade(ORANGE, 0.6f));
  }
  if (s_.drifting && s_.smokeTimer > 0) {
    DrawCircleV({x - ca * 20, y - sa * 20}, 8 + s_.slip * 6, Fade(LIGHTGRAY, 0.35f));
  }
}
