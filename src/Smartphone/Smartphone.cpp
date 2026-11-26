#include "Smartphone.hpp"
#include "Core/FontUI.hpp"
#include <algorithm>
#include "Core/Application.hpp"
#include "Core/Config.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "World/City.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>

Smartphone::Smartphone(Application* app) : app_(app) {}

void Smartphone::Open() { open_ = true; anim_ = 0.f; currentApp_ = PhoneApp::Home; }
void Smartphone::Close() { open_ = false; }

void Smartphone::SetStatus(const std::string& s) {
  statusMsg_ = s;
  statusTimer_ = 2.5f;
}

void Smartphone::Update(float dt) {
  if (!open_) return;
  if (anim_ < 1.f) anim_ = fminf(1.f, anim_ + dt * 4.f);
  if (statusTimer_ > 0) statusTimer_ -= dt;
  if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) Close();
}

bool Smartphone::AppButton(Rectangle r, const char* label, Color accent) {
  Vector2 m = GetMousePosition();
  bool h = CheckCollisionPointRec(m, r);
  DrawRectangleRounded(r, 0.2f, 6, h ? Color{60, 60, 70, 255} : Color{40, 40, 48, 255});
  DrawRectangleRoundedLines(r, 0.2f, 6, accent);
  int tw = FontUI::Get().Measure(label, 16);
  DrawTextRu(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + r.height * 0.5f - 8), 16, WHITE);
  return h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void Smartphone::DrawFrame(Rectangle phone) {
  DrawRectangleRounded(phone, 0.12f, 12, Color{20, 20, 28, 250});
  DrawRectangleRoundedLines(phone, 0.12f, 12, Config::ColAccent());
  // notch
  DrawRectangleRounded({phone.x + phone.width * 0.3f, phone.y + 8, phone.width * 0.4f, 12}, 0.5f, 4, Color{10, 10, 15, 255});
}

void Smartphone::DrawHome(Rectangle c) {
  DrawTextRu("ТЕЛЕФОН", (int)(c.x + 16), (int)(c.y + 8), 18, Config::ColGold());
  const char* names[] = {"Банк", "Криптовалюта", "Даркнет", "Соцсеть", "Такси", "Музыка"};
  PhoneApp apps[] = {PhoneApp::Bank, PhoneApp::Crypto, PhoneApp::Даркнет, PhoneApp::Social, PhoneApp::Taxi, PhoneApp::Music};
  for (int i = 0; i < 6; ++i) {
    float bx = c.x + 16 + (i % 2) * (c.width * 0.5f - 12);
    float by = c.y + 40 + (i / 2) * 70;
    if (AppButton({bx, by, c.width * 0.42f, 56}, names[i], Config::ColAccent()))
      currentApp_ = apps[i];
  }
}

void Smartphone::DrawBank(Rectangle c) {
  auto* p = app_->Player();
  DrawTextRu("БАНК", (int)c.x + 16, (int)c.y + 8, 20, Config::ColText());
  char buf[64];
  snprintf(buf, sizeof(buf), "Баланс: %lld руб.", (long long)p->Money());
  DrawTextRu(buf, (int)c.x + 16, (int)c.y + 40, 18, Config::ColGold());
  if (AppButton({c.x + 16, c.y + 80, c.width - 32, 40}, "Вклад +5% (10к)", Config::ColSuccess())) {
    if (p->Money() >= 10000) {
      p->AddMoney(-10000);
      p->AddMoney(10500);
      SetStatus("Вклад: +500 руб.");
    } else SetStatus("Нужно 10 000");
  }
  if (AppButton({c.x + 16, c.y + 130, c.width - 32, 40}, "Кредит 50к (долг)", Config::ColDanger())) {
    p->AddMoney(50000);
    SetStatus("Кредит получен (вернёшь позже)");
  }
  if (AppButton({c.x + 16, c.y + c.height - 50, c.width - 32, 36}, "Назад", Config::ColMuted()))
    currentApp_ = PhoneApp::Home;
}

void Smartphone::DrawCrypto(Rectangle c) {
  auto* p = app_->Player();
  DrawTextRu("КРИПТО", (int)c.x + 16, (int)c.y + 8, 20, Config::ColText());
  static float btc = 94000.f, eth = 3500.f;
  btc += (rand() % 200 - 100) * 0.5f;
  eth += (rand() % 40 - 20) * 0.2f;
  char buf[80];
  snprintf(buf, sizeof(buf), "BTC ~$%.0f  ETH ~$%.0f", btc, eth);
  DrawTextRu(buf, (int)c.x + 16, (int)c.y + 40, 16, Config::ColMuted());
  if (AppButton({c.x + 16, c.y + 80, c.width - 32, 40}, "Купить BTC (на 20к)", Config::ColAccent())) {
    if (p->Money() >= 20000) {
      p->AddMoney(-20000);
      p->Inv().items.push_back("BTC_dust");
      SetStatus("Купил BTC");
    } else SetStatus("Мало денег");
  }
  if (AppButton({c.x + 16, c.y + 130, c.width - 32, 40}, "Продать BTC (+22к)", Config::ColSuccess())) {
    auto& items = p->Inv().items;
    auto it = std::find(items.begin(), items.end(), "BTC_dust");
    if (it != items.end()) {
      items.erase(it);
      p->AddMoney(22000);
      SetStatus("Продал BTC +22000");
    } else SetStatus("Нет BTC");
  }
  if (AppButton({c.x + 16, c.y + c.height - 50, c.width - 32, 36}, "Назад", Config::ColMuted()))
    currentApp_ = PhoneApp::Home;
}

void Smartphone::DrawДаркнет(Rectangle c) {
  auto* p = app_->Player();
  auto* city = app_->GetWorld()->Current();
  DrawTextRu("ДАРКНЕТ", (int)c.x + 16, (int)c.y + 8, 20, Config::ColDanger());
  bool allowed = city && (city->id == "москва" || city->id == "питер" || city->id == "владивосток");
  if (!allowed) {
    DrawTextRu("Доступно: Москва, Питер, Владивосток", (int)c.x + 12, (int)c.y + 50, 14, Config::ColMuted());
  } else {
    if (AppButton({c.x + 16, c.y + 50, c.width - 32, 40}, "Документы (-2★) 85к", Config::ColAccent())) {
      if (p->Money() >= 85000) {
        p->AddMoney(-85000);
        p->SetWanted(p->Wanted() - 2);
        p->AddHeat(-30);
        SetStatus("Документы получены");
      } else SetStatus("Нужно 85000");
    }
    if (AppButton({c.x + 16, c.y + 100, c.width - 32, 40}, "Сброс тепла 40к", Config::ColAccent())) {
      if (p->Money() >= 40000) {
        p->AddMoney(-40000);
        p->AddHeat(-25);
        SetStatus("Тепло снижено");
      } else SetStatus("Нужно 40000");
    }
    if (AppButton({c.x + 16, c.y + 150, c.width - 32, 40}, "Автомат 185к", Config::ColDanger())) {
      if (p->Money() >= 185000) {
        p->AddMoney(-185000);
        p->Inv().weapons.push_back("Автомат");
        SetStatus("Автомат куплен");
      } else SetStatus("Нужно 185000");
    }
  }
  if (AppButton({c.x + 16, c.y + c.height - 50, c.width - 32, 36}, "Назад", Config::ColMuted()))
    currentApp_ = PhoneApp::Home;
}

void Smartphone::DrawSocial(Rectangle c) {
  DrawTextRu("ЛЕНТА", (int)c.x + 16, (int)c.y + 8, 20, Config::ColText());
  DrawTextRu("Подписчики зависят от тачки\nи одежды в кадре.", (int)c.x + 16, (int)c.y + 40, 16, Config::ColMuted());
  if (AppButton({c.x + 16, c.y + 100, c.width - 32, 40}, "Пост (фото)", Config::ColAccent())) {
    auto* p = app_->Player();
    int bonus = 500 + (int)p->Inv().cars.size() * 800 + p->Level() * 100;
    p->AddMoney(bonus);
    SetStatus("Реклама +" + std::to_string(bonus));
  }
  if (AppButton({c.x + 16, c.y + c.height - 50, c.width - 32, 36}, "Назад", Config::ColMuted()))
    currentApp_ = PhoneApp::Home;
}

void Smartphone::DrawTaxi(Rectangle c) {
  DrawTextRu("ТАКСИ / АВТОПИЛОТ", (int)c.x + 16, (int)c.y + 8, 18, Config::ColText());
  DrawTextRu("Быстрый трансфер по городу", (int)c.x + 16, (int)c.y + 40, 14, Config::ColMuted());
  if (AppButton({c.x + 16, c.y + 80, c.width - 32, 40}, "Вызвать (1500)", Config::ColAccent())) {
    auto* p = app_->Player();
    if (p->Money() >= 1500) {
      p->AddMoney(-1500);
      p->SetEnergy(p->Energy() + 15);
      SetStatus("Такси: энергия +15");
    } else SetStatus("Нужно 1500");
  }
  if (AppButton({c.x + 16, c.y + c.height - 50, c.width - 32, 36}, "Назад", Config::ColMuted()))
    currentApp_ = PhoneApp::Home;
}

void Smartphone::Draw(int sw, int sh) {
  if (!open_) return;
  DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.45f * anim_));

  float pw = 320, ph = 560;
  float px = (sw - pw) * 0.5f;
  float py = (sh - ph) * 0.5f + (1.f - anim_) * 80.f;
  Rectangle phone = {px, py, pw, ph};
  DrawFrame(phone);

  Rectangle content = {px + 14, py + 28, pw - 28, ph - 70};

  switch (currentApp_) {
    case PhoneApp::Home: DrawHome(content); break;
    case PhoneApp::Bank: DrawBank(content); break;
    case PhoneApp::Crypto: DrawCrypto(content); break;
    case PhoneApp::Даркнет: DrawДаркнет(content); break;
    case PhoneApp::Social: DrawSocial(content); break;
    case PhoneApp::Taxi: DrawTaxi(content); break;
    case PhoneApp::Music:
      DrawTextRu("Плеер: музыка", (int)content.x + 16, (int)content.y + 40, 18, Config::ColText());
      if (AppButton({content.x + 16, content.y + content.height - 50, content.width - 32, 36}, "Назад", Config::ColMuted()))
        currentApp_ = PhoneApp::Home;
      break;
    default: break;
  }

  if (statusTimer_ > 0) {
    DrawTextRu(statusMsg_.c_str(), (int)(px + 20), (int)(py + ph - 36), 14, Config::ColGold());
  }
}
