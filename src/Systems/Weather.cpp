#include "Weather.hpp"
#include <cmath>
#include <cstdlib>

void WeatherSystem::Set(WeatherType t) {
  type_ = t;
  rain_.clear();
}

std::string WeatherSystem::Name() const {
  switch (type_) {
    case WeatherType::Clear: return "Ясно";
    case WeatherType::Rain: return "Дождь";
    case WeatherType::NeonNight: return "Неоновая ночь";
    case WeatherType::Fog: return "Туман";
  }
  return "?";
}

float WeatherSystem::TractionMult() const {
  if (type_ == WeatherType::Rain) return 0.72f;
  if (type_ == WeatherType::Fog) return 0.85f;
  return 1.f;
}

void WeatherSystem::Update(float dt, float timeOfDay) {
  timer_ += dt;
  // auto cycle occasionally
  if (timer_ > 90.f) {
    timer_ = 0.f;
    int r = rand() % 100;
    if (timeOfDay > 21.f || timeOfDay < 5.f) {
      type_ = (r < 40) ? WeatherType::NeonNight : ((r < 70) ? WeatherType::Rain : WeatherType::Clear);
    } else if (r < 25) type_ = WeatherType::Rain;
    else if (r < 35) type_ = WeatherType::Fog;
    else type_ = WeatherType::Clear;
    rain_.clear();
  }
  if (type_ == WeatherType::Rain) {
    for (auto& d : rain_) {
      d.y += d.spd * dt * 60.f;
    }
  }
}

void WeatherSystem::EnsureRain(int sw, int sh) {
  if (rain_.size() >= 120) return;
  while ((int)rain_.size() < 120) {
    rain_.push_back({(float)(rand() % sw), (float)(rand() % sh), 8.f + (rand() % 10)});
  }
}

void WeatherSystem::Draw(int sw, int sh) {
  if (type_ == WeatherType::Rain) {
    EnsureRain(sw, sh);
    for (auto& d : rain_) {
      if (d.y > sh) { d.y = -10; d.x = (float)(rand() % sw); }
      DrawLineEx({d.x, d.y}, {d.x - 1.f, d.y + 12.f}, 1.5f, Color{160, 180, 220, 140});
    }
    DrawRectangle(0, 0, sw, sh, Fade(Color{30, 40, 60, 255}, 0.12f));
  } else if (type_ == WeatherType::Fog) {
    DrawRectangle(0, 0, sw, sh, Fade(Color{180, 180, 190, 255}, 0.18f));
  } else if (type_ == WeatherType::NeonNight) {
    DrawRectangle(0, 0, sw, sh / 2, Fade(Color{80, 0, 60, 255}, 0.15f));
  }
}
