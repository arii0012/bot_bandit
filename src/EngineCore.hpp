#pragma once
#include "raylib.h"
#include "raymath.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>

namespace fs = std::filesystem;
using json = nlohmann::json;

// ---------- EngineOptimizer ----------
struct AABB {
  Vector3 min{}, max{};
  bool Contains(Vector3 p) const {
    return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y && p.z >= min.z && p.z <= max.z;
  }
  Vector3 Center() const { return Vector3Scale(Vector3Add(min, max), 0.5f); }
};

class EngineOptimizer {
public:
  static bool InFrustum(const Camera3D& cam, Vector3 pos, float radius) {
    Vector3 to = Vector3Subtract(pos, cam.position);
    float dist = Vector3Length(to);
    if (dist < 0.5f) return true;
    if (dist > 180.f) return false;
    Vector3 fwd = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
    float cosA = Vector3DotProduct(Vector3Normalize(to), fwd);
    float limit = cosf((cam.fovy + 25.f) * 0.5f * DEG2RAD);
    return cosA > limit - radius / dist;
  }

  static int LodLevel(float dist) {
    if (dist < 25.f) return 0;
    if (dist < 60.f) return 1;
    if (dist < 120.f) return 2;
    return 3;
  }

  void BeginFrame() { drawCalls_ = 0; culled_ = 0; }
  void NoteDraw() { drawCalls_++; }
  void NoteCull() { culled_++; }
  int DrawCalls() const { return drawCalls_; }
  int Culled() const { return culled_; }

  // garage streaming: only active hall index
  int ActiveHall() const { return hall_; }
  void SetHall(int h) { hall_ = std::max(0, h); }

private:
  int drawCalls_ = 0, culled_ = 0, hall_ = 0;
};

// ---------- PhysicsWorld ----------
struct RigidBody {
  Vector3 pos{};
  Vector3 vel{};
  Vector3 size{1, 1, 1};
  float mass = 1.f;
  float yaw = 0.f;
  float angularVel = 0.f;
  float health = 100.f;
  bool isStatic = false;
  bool isCar = false;
};

class PhysicsWorld {
public:
  int Add(const RigidBody& b) { bodies_.push_back(b); return (int)bodies_.size() - 1; }
  RigidBody& Get(int i) { return bodies_[i]; }
  const std::vector<RigidBody>& All() const { return bodies_; }

  static bool AabbOverlap(const RigidBody& a, const RigidBody& b) {
    Vector3 amin = Vector3Subtract(a.pos, Vector3Scale(a.size, 0.5f));
    Vector3 amax = Vector3Add(a.pos, Vector3Scale(a.size, 0.5f));
    Vector3 bmin = Vector3Subtract(b.pos, Vector3Scale(b.size, 0.5f));
    Vector3 bmax = Vector3Add(b.pos, Vector3Scale(b.size, 0.5f));
    return amin.x <= bmax.x && amax.x >= bmin.x && amin.y <= bmax.y && amax.y >= bmin.y &&
           amin.z <= bmax.z && amax.z >= bmin.z;
  }

  void SetWet(bool w) { wet_ = w; }

  void Update(float dt) {
    for (auto& b : bodies_) {
      if (b.isStatic) continue;
      float drag = wet_ ? 0.92f : 0.96f;
      b.vel = Vector3Scale(b.vel, std::pow(drag, dt * 60.f));
      b.pos = Vector3Add(b.pos, Vector3Scale(b.vel, dt));
      b.yaw += b.angularVel * dt;
      b.angularVel *= (wet_ ? 0.9f : 0.95f);
      // ground
      float halfH = b.size.y * 0.5f;
      if (b.pos.y < halfH) {
        b.pos.y = halfH;
        if (b.vel.y < 0) b.vel.y = 0;
      }
    }
    // collisions
    for (size_t i = 0; i < bodies_.size(); ++i) {
      for (size_t j = i + 1; j < bodies_.size(); ++j) {
        auto& a = bodies_[i];
        auto& b = bodies_[j];
        if (!AabbOverlap(a, b)) continue;
        Vector3 n = Vector3Subtract(b.pos, a.pos);
        float len = Vector3Length(n);
        if (len < 0.001f) n = {1, 0, 0};
        else n = Vector3Scale(n, 1.f / len);
        float invA = a.isStatic ? 0.f : 1.f / a.mass;
        float invB = b.isStatic ? 0.f : 1.f / b.mass;
        Vector3 rel = Vector3Subtract(b.vel, a.vel);
        float sep = Vector3DotProduct(rel, n);
        if (sep > 0) continue;
        float e = 0.25f;
        float jimp = -(1.f + e) * sep / (invA + invB + 0.0001f);
        Vector3 impulse = Vector3Scale(n, jimp);
        if (!a.isStatic) {
          a.vel = Vector3Subtract(a.vel, Vector3Scale(impulse, invA));
          if (a.isCar) a.health = std::max(0.f, a.health - std::abs(jimp) * 0.02f);
        }
        if (!b.isStatic) {
          b.vel = Vector3Add(b.vel, Vector3Scale(impulse, invB));
          if (b.isCar) b.health = std::max(0.f, b.health - std::abs(jimp) * 0.02f);
        }
        // positional correction
        float overlap = 0.1f;
        if (!a.isStatic) a.pos = Vector3Subtract(a.pos, Vector3Scale(n, overlap * 0.5f));
        if (!b.isStatic) b.pos = Vector3Add(b.pos, Vector3Scale(n, overlap * 0.5f));
      }
    }
  }

  void ApplyDrive(int id, float throttle, float steer, float dt, bool handbrake) {
    if (id < 0 || id >= (int)bodies_.size()) return;
    auto& b = bodies_[id];
    float grip = wet_ ? 0.65f : 1.f;
    if (handbrake) grip *= 0.4f;
    float yawRad = b.yaw * DEG2RAD;
    Vector3 fwd = {sinf(yawRad), 0, cosf(yawRad)};
    b.vel = Vector3Add(b.vel, Vector3Scale(fwd, throttle * 12.f * grip * dt));
    float speed = Vector3Length(b.vel);
    b.angularVel += steer * speed * 0.8f * grip * dt;
    if (handbrake && speed > 5.f) {
      Vector3 side = {-fwd.z, 0, fwd.x};
      b.vel = Vector3Add(b.vel, Vector3Scale(side, steer * speed * 0.15f));
    }
  }

private:
  std::vector<RigidBody> bodies_;
  bool wet_ = false;
};

// ---------- ParticleSystem3D ----------
struct Particle3D {
  Vector3 pos{}, vel{};
  float life = 0, maxLife = 1;
  float size = 0.2f;
  Color color{255, 255, 255, 255};
  bool alive = false;
};

class ParticleSystem3D {
public:
  void EmitSmoke(Vector3 p, int n = 4) {
    for (int i = 0; i < n; ++i) Spawn(p, {Rand(-0.5f, 0.5f), Rand(0.5f, 1.5f), Rand(-0.5f, 0.5f)},
                                       {120, 120, 120, 180}, 0.4f, Rand(0.4f, 0.9f));
  }
  void EmitExhaust(Vector3 p, bool nitro) {
    Color c = nitro ? Color{255, 140, 40, 200} : Color{80, 80, 80, 160};
    Spawn(p, {Rand(-0.2f, 0.2f), Rand(0.2f, 0.8f), Rand(-0.2f, 0.2f)}, c, 0.25f, 0.5f);
  }
  void EmitSparks(Vector3 p) {
    for (int i = 0; i < 6; ++i)
      Spawn(p, {Rand(-2, 2), Rand(1, 4), Rand(-2, 2)}, {255, 200, 50, 255}, 0.08f, 0.35f);
  }
  void EmitRain(Vector3 center, float radius) {
    for (int i = 0; i < 8; ++i) {
      Vector3 p = {center.x + Rand(-radius, radius), center.y + 8.f, center.z + Rand(-radius, radius)};
      Spawn(p, {0, -12.f, 0}, {180, 200, 255, 120}, 0.05f, 0.8f);
    }
  }
  void EmitMuzzle(Vector3 p) {
    Spawn(p, {0, 0.2f, 0}, {255, 240, 150, 255}, 0.3f, 0.12f);
  }
  void EmitConfetti(Vector3 p) {
    for (int i = 0; i < 20; ++i) {
      Color c = {(unsigned char)GetRandomValue(50, 255), (unsigned char)GetRandomValue(50, 255),
                 (unsigned char)GetRandomValue(50, 255), 255};
      Spawn(p, {Rand(-3, 3), Rand(2, 6), Rand(-3, 3)}, c, 0.12f, 1.5f);
    }
  }
  void EmitHoodSmoke(Vector3 p) {
    for (int i = 0; i < 3; ++i)
      Spawn(p, {Rand(-0.3f, 0.3f), Rand(0.8f, 1.5f), Rand(-0.3f, 0.3f)}, {40, 40, 40, 200}, 0.35f, 1.2f);
  }

  void Update(float dt) {
    for (auto& p : pool_) {
      if (!p.alive) continue;
      p.life -= dt;
      if (p.life <= 0) { p.alive = false; continue; }
      p.pos = Vector3Add(p.pos, Vector3Scale(p.vel, dt));
      p.vel.y += -2.f * dt;
      p.size += dt * 0.15f;
    }
  }

  void Draw() {
    for (auto& p : pool_) {
      if (!p.alive) continue;
      float a = p.life / p.maxLife;
      Color c = p.color;
      c.a = (unsigned char)(c.a * a);
      DrawSphere(p.pos, p.size, c);
    }
  }

private:
  static constexpr int MAX = 5120;
  Particle3D pool_[MAX]{};
  int cursor_ = 0;
  static float Rand(float a, float b) {
    return a + (b - a) * (GetRandomValue(0, 10000) / 10000.f);
  }
  void Spawn(Vector3 pos, Vector3 vel, Color col, float size, float life) {
    auto& p = pool_[cursor_++ % MAX];
    p.pos = pos; p.vel = vel; p.color = col; p.size = size;
    p.life = p.maxLife = life; p.alive = true;
  }
};

// ---------- SaveSystemAtomic ----------
class SaveSystemAtomic {
public:
  explicit SaveSystemAtomic(std::string path = "data/save.json") : path_(std::move(path)) {}

  bool Exists() const { return fs::exists(path_); }

  bool Save(const json& data) {
    try {
      fs::create_directories(fs::path(path_).parent_path());
      std::string tmp = path_ + ".tmp";
      {
        std::ofstream o(tmp);
        if (!o) return false;
        o << data.dump(2);
      }
      std::error_code ec;
      fs::rename(tmp, path_, ec);
      if (ec) {
        fs::copy_file(tmp, path_, fs::copy_options::overwrite_existing, ec);
        fs::remove(tmp, ec);
      }
      return true;
    } catch (...) {
      return false;
    }
  }

  bool Load(json& out) {
    try {
      if (!fs::exists(path_)) return false;
      std::ifstream i(path_);
      if (!i) return false;
      i >> out;
      return true;
    } catch (...) {
      // try backup
      try {
        std::string bak = path_ + ".tmp";
        if (fs::exists(bak)) {
          std::ifstream i(bak);
          i >> out;
          return true;
        }
      } catch (...) {}
      return false;
    }
  }

  json MakePlayerBlob(
      const std::string& name, int64_t money, float crypto, int level, int energy, int heat, int wanted,
      const std::string& city, const std::vector<std::string>& cars,
      const std::vector<std::string>& clothes, const std::map<std::string, int>& skills,
      const std::string& activeCar, int engine, int turbo, bool neon,
      unsigned char cr, unsigned char cg, unsigned char cb,
      const std::vector<std::string>& storyDone) {
    json j;
    j["name"] = name;
    j["money"] = money;
    j["crypto"] = crypto;
    j["level"] = level;
    j["energy"] = energy;
    j["heat"] = heat;
    j["wanted"] = wanted;
    j["city"] = city;
    j["cars"] = cars;
    j["clothes"] = clothes;
    j["skills"] = skills;
    j["activeCar"] = activeCar;
    j["engine"] = engine;
    j["turbo"] = turbo;
    j["neon"] = neon;
    j["carColor"] = {cr, cg, cb};
    j["story"] = storyDone;
    return j;
  }

private:
  std::string path_;
};
