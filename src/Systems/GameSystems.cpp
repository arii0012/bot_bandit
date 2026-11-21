#include "GameSystems.hpp"
#include "Core/Application.hpp"
#include "UI/UISystem.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "World/City.hpp"
#include <cstdlib>
#include <ctime>

GameSystems::GameSystems(Application* app) : app_(app) {
  srand((unsigned)time(nullptr));
}

void GameSystems::Update(float dt) {
  if (workCd_ > 0) workCd_ -= dt;
  if (robCd_ > 0) robCd_ -= dt;
  if (heistCd_ > 0) heistCd_ -= dt;
  if (dailyCd_ > 0) dailyCd_ -= dt;
}

void GameSystems::DoWork(int jobIndex) {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  auto* city = app_->GetWorld()->Current();
  if (!city || city->jobs.empty()) { ui->Toast("Нет работы"); return; }
  if (p->Energy() < 12) { ui->Toast("Нет энергии"); return; }
  if (workCd_ > 0) { ui->Toast("Подожди"); return; }

  if (jobIndex < 0 || jobIndex >= (int)city->jobs.size()) jobIndex = 0;
  auto& job = city->jobs[jobIndex];
  float mult = 1.f + city->workBonus / 200.f;
  int earned = (int)((job.basePay + p->Level() * 220 + p->Strength() * 160) * mult);
  p->AddMoney(earned);
  p->SetEnergy(p->Energy() - 12);
  workCd_ = 12.f;
  auto msgs = p->AddExp(25);
  ui->Toast("+" + std::to_string(earned) + " руб. (" + job.name + ")");
}

void GameSystems::DoRob() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  auto* city = app_->GetWorld()->Current();
  if (p->Energy() < 18) { ui->Toast("Нет энергии"); return; }
  if (robCd_ > 0) { ui->Toast("Слишком горячо"); return; }
  if (p->Wanted() >= 4) { ui->Toast("Розыск слишком высокий"); return; }

  float chance = 0.55f + p->Skill("stealth") * 0.05f - city->danger * 0.04f;
  p->SetEnergy(p->Energy() - 15);
  robCd_ = 25.f;
  if ((rand() % 1000) / 1000.f < chance) {
    int amount = 1800 + p->Level() * 350 + city->danger * 900 + rand() % 2000;
    p->AddMoney(amount);
    p->AddHeat(8 + city->danger * 3);
    p->AddExp(35);
    ui->Toast("Ограбление +" + std::to_string(amount));
  } else {
    p->AddHeat(15 + city->danger * 4);
    ui->Toast("Провал! Тепло выросло");
  }
}

void GameSystems::DoHeist() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (p->Level() < 2) { ui->Toast("Нужен 2 уровень"); return; }
  if (p->Energy() < 35) { ui->Toast("Нужно 35 энергии"); return; }
  if (heistCd_ > 0) { ui->Toast("КД налёта"); return; }

  ui->Modal("СЕРЬЁЗНОЕ ОГРАБЛЕНИЕ", {
    "Киоск (низкий риск)",
    "Ювелирка",
    "Банк",
    "Инкассаторы",
    "Отмена"
  }, [this](int choice) {
    if (choice < 0 || choice > 3) return;
    auto* p = app_->Player();
    auto* ui = app_->UI();
    auto* city = app_->GetWorld()->Current();
    struct T { float ch; int mn, mx, heat; };
    T targets[] = {
      {0.78f, 25000, 70000, 10},
      {0.55f, 90000, 220000, 22},
      {0.38f, 200000, 550000, 35},
      {0.28f, 400000, 1200000, 48}
    };
    auto t = targets[choice];
    float chance = t.ch + p->Skill("stealth") * 0.04f + p->Skill("shooting") * 0.02f
                   - p->Wanted() * 0.06f - city->danger * 0.02f;
    p->SetEnergy(p->Energy() - 30);
    heistCd_ = 90.f;
    if ((rand() % 1000) / 1000.f < chance) {
      int amount = t.mn + rand() % (t.mx - t.mn + 1);
      p->AddMoney(amount);
      p->AddHeat(t.heat);
      p->AddExp(70);
      ui->Toast("УСПЕХ +" + std::to_string(amount));
    } else {
      p->AddHeat(t.heat + 18);
      p->SetWanted(p->Wanted() + 1);
      ui->Toast("ПРОВАЛ налёта");
    }
  });
}

void GameSystems::OpenTravel() {
  auto* ui = app_->UI();
  std::vector<std::string> opts;
  for (auto& c : AllCities()) opts.push_back(c.name);
  opts.push_back("Отмена");
  ui->Modal("КУДА ЕДЕМ?", opts, [this](int i) {
    auto& list = AllCities();
    if (i < 0 || i >= (int)list.size()) return;
    auto* p = app_->Player();
    auto* w = app_->GetWorld();
    int cost = 3000 + abs((int)(list[i].mapX * 1000) - (int)(w->Current()->mapX * 1000)) * 20;
    if (p->Money() < cost) { app_->UI()->Toast("Нужно " + std::to_string(cost)); return; }
    p->AddMoney(-cost);
    p->SetEnergy(p->Energy() - 8);
    w->SetCity(list[i].id);
    p->SetCity(list[i].id);
    p->AddExp(15);
    app_->UI()->Toast("Прибыл: " + list[i].name);
  });
}

void GameSystems::OpenShop() {
  app_->UI()->Modal("МАГАЗИН", {
    "Нож — 8 000",
    "Пистолет — 45 000",
    "Энергетик — 1 200",
    "Канистра — 3 500",
    "Отмена"
  }, [this](int i) {
    auto* p = app_->Player();
    auto* ui = app_->UI();
    if (i == 0 && p->Money() >= 8000) { p->AddMoney(-8000); p->Inv().weapons.push_back("Нож"); ui->Toast("Купил нож"); }
    else if (i == 1 && p->Money() >= 45000) { p->AddMoney(-45000); p->Inv().weapons.push_back("Пистолет"); ui->Toast("Купил пистолет"); }
    else if (i == 2 && p->Money() >= 1200) { p->AddMoney(-1200); p->SetEnergy(p->Energy() + 40); ui->Toast("Энергия +40"); }
    else if (i == 3 && p->Money() >= 3500) { p->AddMoney(-3500); ui->Toast("Топливо +30"); }
    else if (i < 4) ui->Toast("Не хватает денег");
  });
}

void GameSystems::DoScavenge() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (p->Energy() < 12) { ui->Toast("Нет энергии"); return; }
  p->SetEnergy(p->Energy() - 10);
  const char* mats[] = {"scrap", "electronics", "chemicals", "fabric", "metal"};
  std::string msg = "Найдено: ";
  for (int i = 0; i < 2 + p->Intelligence() / 3; ++i) {
    auto* m = mats[rand() % 5];
    int q = 1 + rand() % 3;
    p->Inv().materials[m] += q;
    msg += std::string(m) + "x" + std::to_string(q) + " ";
  }
  p->AddExp(8);
  ui->Toast(msg);
}

void GameSystems::DoDaily() {
  if (dailyCd_ > 0) { app_->UI()->Toast("Уже получено"); return; }
  auto* p = app_->Player();
  int reward = 3500 + p->Level() * 400 + rand() % 1500;
  p->AddMoney(reward);
  p->SetEnergy(100);
  dailyCd_ = 60.f * 20; // short for demo; real would be 20h
  p->AddExp(50);
  app_->UI()->Toast("Ежедневка +" + std::to_string(reward));
}

void GameSystems::DoHide() {
  auto* p = app_->Player();
  if (p->Energy() < 8) { app_->UI()->Toast("Нет сил"); return; }
  p->SetEnergy(p->Energy() - 8);
  p->AddHeat(-(12 + rand() % 10));
  if (p->Heat() < 25 && p->Wanted() > 0 && rand() % 100 < 40)
    p->SetWanted(p->Wanted() - 1);
  app_->UI()->Toast("Спрятался. Тепло: " + std::to_string(p->Heat()));
}

void GameSystems::DoCraft(const std::string& recipeId) {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (recipeId == "medkit") {
    if (p->Inv().materials["fabric"] >= 2 && p->Inv().materials["chemicals"] >= 2) {
      p->Inv().materials["fabric"] -= 2;
      p->Inv().materials["chemicals"] -= 2;
      p->Inv().items.push_back("Аптечка");
      ui->Toast("Скрафтил аптечку");
    } else ui->Toast("Нужно fabric×2 chemicals×2");
  } else if (recipeId == "lockpick") {
    if (p->Inv().materials["metal"] >= 2 && p->Inv().materials["scrap"] >= 3) {
      p->Inv().materials["metal"] -= 2;
      p->Inv().materials["scrap"] -= 3;
      p->Inv().items.push_back("Отмычка");
      ui->Toast("Скрафтил отмычку");
    } else ui->Toast("Нужно metal×2 scrap×3");
  } else {
    ui->Toast("Рецепт: medkit | lockpick");
  }
}
