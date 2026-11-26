#include "GameSystems.hpp"
#include "Core/HotReloadConfig.hpp"
#include "Systems/DifficultySystem.hpp"
#include "Systems/AntiAbuse.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Core/EventBus.hpp"
#include <algorithm>
#include "Core/Application.hpp"
#include "UI/UISystem.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "World/City.hpp"
#include "Crafting.hpp"
#include "Vehicles/Garage.hpp"
#include "Business.hpp"
#include "Cutscenes/CutsceneSystem.hpp"
#include "Minigames/Fighting.hpp"
#include "Minigames/Racing.hpp"
#include "Minigames/DJ.hpp"
#include "SmartHome.hpp"
#include "Vehicles/CarCustomizer.hpp"
#include "Vehicles/Auction.hpp"
#include "Systems/Weather.hpp"
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
  int earned = (int)((job.basePay + p->Level() * 220 + p->Strength() * 160) * mult * DifficultySystem::Get().QuestPayMult() * HotReloadConfig::Get().WorkPayMult());
  p->AddMoney(earned);
  p->SetEnergy(p->Energy() - 12);
  workCd_ = 12.f;
  auto msgs = p->AddExp(25);
  ui->Toast("+" + ArtworkRenderer::FormatRub(earned) + " (" + job.name + ")");
  EventBus::Get().Emit(GameEvent::MoneyChanged, EventPayload{earned});
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
      if (app_->Cutscenes()) app_->Cutscenes()->Play("heist_win");
    } else {
      p->AddHeat(t.heat + 18);
      p->SetWanted(p->Wanted() + 1);
      ui->Toast("ПРОВАЛ налёта");
      if (app_->Cutscenes()) app_->Cutscenes()->Play("arrest");
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
    w->SyncOwnedHouses(*p);
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
  std::string msg;
  CraftingSystem::Craft(*p, recipeId, msg);
  ui->Toast(msg);
}

void GameSystems::OpenCraft() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  std::vector<std::string> opts;
  std::vector<std::string> ids;
  for (auto& r : CraftingSystem::Recipes()) {
    std::string reason;
    bool ok = CraftingSystem::CanCraft(*p, r, reason);
    std::string line = CraftingSystem::FormatRecipe(r);
    if (!ok) line += " (" + reason + ")";
    opts.push_back(line);
    ids.push_back(r.id);
  }
  opts.push_back("Поиск материалов (scavenge)");
  opts.push_back("Отмена");
  ui->Modal("КРАФТ", opts, [this, ids](int i) {
    if (i < 0) return;
    if (i == (int)ids.size()) { DoScavenge(); return; }
    if (i > (int)ids.size()) return;
    DoCraft(ids[i]);
  });
}

void GameSystems::OpenGarage() {
  auto* ui = app_->UI();
  std::vector<std::string> opts;
  std::vector<std::string> ids;
  for (auto& c : Garage::Catalog()) {
    opts.push_back(c.name + " — " + std::to_string(c.price) + " руб.");
    ids.push_back(c.id);
  }
  opts.push_back("Тюнинг: двигатель");
  opts.push_back("Тюнинг: турбо");
  opts.push_back("Тюнинг: неон");
  opts.push_back("Тюнинг: покраска");
  opts.push_back("Отмена");
  int nCars = (int)ids.size();
  ui->Modal("ГАРАЖ / АВТОСАЛОН", opts, [this, ids, nCars](int i) {
    auto* p = app_->Player();
    auto* ui = app_->UI();
    std::string msg;
    if (i < 0 || i >= nCars + 4) return;
    if (i < nCars) {
      Garage::Buy(*p, ids[i], msg);
      if (msg.find("Купил") != std::string::npos && app_->Cutscenes())
        app_->Cutscenes()->Play("keys");
    } else if (i == nCars) Garage::Tune(*p, "engine", msg);
    else if (i == nCars + 1) Garage::Tune(*p, "turbo", msg);
    else if (i == nCars + 2) Garage::Tune(*p, "neon", msg);
    else if (i == nCars + 3) Garage::Tune(*p, "paint", msg);
    ui->Toast(msg);
  });
}

void GameSystems::OpenBusiness() {
  auto* ui = app_->UI();
  ui->Modal("БИЗНЕС", {
    "Купить / апгрейд точку",
    "Собрать доход",
    "Отмена"
  }, [this](int i) {
    auto* p = app_->Player();
    auto* w = app_->GetWorld();
    auto* ui = app_->UI();
    std::string msg;
    if (i == 0) BusinessSystem::BuyOrUpgrade(*p, *w, msg);
    else if (i == 1) BusinessSystem::Collect(*p, *w, msg);
    else return;
    ui->Toast(msg);
  });
}

void GameSystems::PlayCutscene(const std::string& id) {
  if (app_->Cutscenes()) app_->Cutscenes()->Play(id);
}


void GameSystems::DoFight() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  auto* city = app_->GetWorld()->Current();
  if (!city || (!city->hasClub && city->id != "москва" && city->id != "ростов" && city->id != "екатеринбург")) {
    ui->Toast("Бойцовский клуб недоступен здесь");
    return;
  }
  ui->Modal("БОЙЦОВСКИЙ КЛУБ", {"Ставка 5к", "Ставка 20к", "Ставка 50к", "Отмена"}, [this](int i) {
    if (i < 0 || i > 2) return;
    int stakes[] = {5000, 20000, 50000};
    auto msg = FightingClub::Fight(*app_->Player(), stakes[i]);
    app_->UI()->Toast(msg);
  });
}

void GameSystems::BuyPet() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (p->Money() < 120000) { ui->Toast("Доберман стоит 120к"); return; }
  for (auto& it : p->Inv().items) if (it == "Доберман") { ui->Toast("Уже есть пёс"); return; }
  p->AddMoney(-120000);
  p->Inv().items.push_back("Доберман");
  ui->Toast("Доберман охраняет дом (+защита)");
}

void GameSystems::DoSmuggle() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (p->ActiveCar().empty()) { ui->Toast("Нужна машина"); return; }
  if (p->Energy() < 20) { ui->Toast("Нет энергии"); return; }
  ui->Modal("КОНТРАБАНДА", {
    "Груз на юг (риск)",
    "Груз во Владивосток",
    "Отмена"
  }, [this](int i) {
    if (i < 0 || i > 1) return;
    auto* p = app_->Player();
    auto* ui = app_->UI();
    p->SetEnergy(p->Energy() - 18);
    float chance = 0.55f + p->Skill("driving") * 0.05f + p->turboLvl_ * 0.03f;
    if ((rand()%1000)/1000.f < chance) {
      int pay = 40000 + p->Level() * 5000 + (i == 1 ? 30000 : 0);
      p->AddMoney(pay);
      p->AddExp(35);
      ui->Toast("Груз доставлен +" + std::to_string(pay));
    } else {
      p->AddHeat(20);
      p->SetWanted(p->Wanted() + 1);
      ui->Toast("Пост ГАИ! Груз потерян");
      if (app_->Cutscenes()) app_->Cutscenes()->Play("arrest");
    }
  });
}

void GameSystems::DoRace() {
  auto* ui = app_->UI();
  ui->Modal("СТРИТРЕСИНГ", {
    "Гонка на деньги",
    "PINK SLIP (на тачку)",
    "Отмена"
  }, [this](int i) {
    if (i < 0 || i > 1) return;
    auto msg = StreetRace::Race(*app_->Player(), app_->Weather(), i == 1);
    app_->UI()->Toast(msg);
  });
}

void GameSystems::BuyMansion() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (p->Inv().materials["house_mansion"] > 0) { ui->Toast("Особняк уже есть"); return; }
  int cost = 8500000;
  if (p->Money() < cost) { ui->Toast("Особняк: 8.5 млн"); return; }
  p->AddMoney(-cost);
  p->Inv().materials["house_mansion"] = 1;
  p->Inv().houses.push_back("Особняк");
  ui->Toast("Ключи от особняка!");
  if (app_->Cutscenes()) app_->Cutscenes()->Play("mansion");
}

void GameSystems::OpenSmartHome() {
  auto* ui = app_->UI();
  ui->Modal("УМНЫЙ ДОМ", {
    "Свет", "Сигнализация", "Сейф", "Вечеринка",
    "Заказ еды", "Заказ оружия", "Купить особняк", "Отмена"
  }, [this](int i) {
    auto* p = app_->Player();
    auto* ui = app_->UI();
    std::string msg;
    switch (i) {
      case 0: msg = SmartHome::ToggleLight(*p); break;
      case 1: msg = SmartHome::ToggleAlarm(*p); break;
      case 2: msg = SmartHome::OpenSafe(*p); break;
      case 3: msg = SmartHome::Party(*p); break;
      case 4: msg = SmartHome::OrderGear(*p, "food"); break;
      case 5: msg = SmartHome::OrderGear(*p, "weapon"); break;
      case 6: BuyMansion(); return;
      default: return;
    }
    ui->Toast(msg);
  });
}

void GameSystems::OpenStudio() {
  auto* ui = app_->UI();
  ui->Modal("МУЗЫКАЛЬНАЯ СТУДИЯ", {
    "Записать трек (8к)",
    "Собрать роялти",
    "Отмена"
  }, [this](int i) {
    auto* p = app_->Player();
    std::string msg;
    if (i == 0) msg = MusicStudio::ProduceTrack(*p);
    else if (i == 1) msg = MusicStudio::CollectRoyalties(*p);
    else return;
    app_->UI()->Toast(msg);
  });
}

void GameSystems::DoAutopilotTravel() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  std::string car = p->ActiveCar();
  bool ap = car.find("Tesla") != std::string::npos || car.find("Mercedes") != std::string::npos;
  if (!ap) { ui->Toast("Автопилот: только Tesla / Mercedes"); return; }
  // open travel with discount flag via cheaper path
  auto& list = AllCities();
  std::vector<std::string> opts;
  for (auto& c : list) opts.push_back(c.name + " (автопилот)");
  opts.push_back("Отмена");
  ui->Modal("АВТОПИЛОТ", opts, [this](int i) {
    auto& list = AllCities();
    if (i < 0 || i >= (int)list.size()) return;
    auto* p = app_->Player();
    auto* w = app_->GetWorld();
    int cost = 1500 + abs((int)(list[i].mapX * 800) - (int)(w->Current()->mapX * 800));
    if (p->Money() < cost) { app_->UI()->Toast("Нужно " + std::to_string(cost)); return; }
    p->AddMoney(-cost);
    p->SetEnergy(std::min(100, p->Energy() + 5));
    w->SetCity(list[i].id);
    p->SetCity(list[i].id);
    app_->UI()->Toast("Автопилот: " + list[i].name);
    if (app_->Cutscenes()) app_->Cutscenes()->Play("travel");
  });
}

void GameSystems::OpenCarCustom() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (p->ActiveCar().empty()) { ui->Toast("Сначала купи/выбери авто (8)"); return; }
  ui->Modal("КАСТОМ АВТО", {
    "Покраска мат (45к)",
    "Металлик (75к)",
    "Хамелеон (150к)",
    "Неон (80к)",
    "Спойлер (55к)",
    "Диски (40к)",
    "Винил (60к)",
    "Подвеска (90к)",
    "Ксенон (35к)",
    "Номер (15к)",
    "Шины Soft (95к)",
    "Шины Medium (70к)",
    "Шины Hard (55к)",
    "Обвес bodykit (180к)",
    "Отмена"
  }, [this](int i) {
    const char* opts[] = {"paint_mat","paint_metal","paint_chameleon","neon","spoiler","wheels","vinyl","suspension","xenon","plate","tires_soft","tires_med","tires_hard","bodykit"};
    if (i < 0 || i > 13) return;
    std::string msg;
    CarCustomizer::Apply(*app_->Player(), opts[i], msg);
    app_->UI()->Toast(msg);
  });
}

void GameSystems::StartDriftMode() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (p->ActiveCar().empty()) { ui->Toast("Нужна машина"); return; }
  ui->Toast("ДРИФТ: WASD/стрелки газ-поворот, SPACE ручник, SHIFT nitro, F выход");
  // flag via materials
  p->Inv().materials["drift_mode"] = 1;
}

void GameSystems::OpenAuction() {
  auto lots = CarAuction::CurrentLots();
  std::vector<std::string> opts;
  for (auto& L : lots)
    opts.push_back(L.name + " — " + std::to_string(L.price) + "р");
  opts.push_back("Отмена");
  app_->UI()->Modal("АУКЦИОН КОНТЕЙНЕРОВ", opts, [this, n=(int)lots.size()](int i) {
    if (i < 0 || i >= n) return;
    app_->UI()->Toast(CarAuction::BuyLot(*app_->Player(), i));
  });
}

void GameSystems::RepairProject() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  for (auto& it : p->Inv().items) {
    if (it.rfind("project:", 0) != 0) continue;
    std::string id = it.substr(8);
    int cost = p->Inv().materials["repair_" + id];
    if (cost <= 0) cost = 200000;
    if (p->Money() < cost) { ui->Toast("Ремонт: " + std::to_string(cost)); return; }
    p->AddMoney(-cost);
    p->Inv().cars.push_back("Restored " + id);
    p->SetActiveCar("Restored " + id);
    p->Inv().materials["repair_" + id] = 0;
    ui->Toast("Реставрация готова!");
    if (app_->Cutscenes()) app_->Cutscenes()->Play("keys");
    return;
  }
  ui->Toast("Нет проектных авто (сначала аукцион)");
}


void GameSystems::DoClub() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  int fee = HotReloadConfig::Get().ClubEntry();
  if (p->Money() < fee) { ui->Toast("Club entry"); return; }
  p->AddMoney(-fee);
  p->SetEnergy(std::min(100, p->Energy() + 25));
  ui->Toast("Club: energy +25");
  if (app_->Cutscenes()) app_->Cutscenes()->Play("vip_club");
}

void GameSystems::DoClubVip() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  bool dress = false;
  for (auto& c : p->Inv().clothes) {
    if (c.find("suit") != std::string::npos || c.find("chain") != std::string::npos ||
        c.find("gold") != std::string::npos) dress = true;
  }
  if (!dress) { ui->Toast("VIP dress-code failed"); return; }
  int fee = HotReloadConfig::Get().VipEntry();
  if (p->Money() < fee) { ui->Toast("VIP entry"); return; }
  p->AddMoney(-fee);
  p->SetEnergy(100);
  p->AddHeat(-5);
  ui->Toast("VIP lounge: energy full, heat -5");
}

void GameSystems::DoCasino(int game) {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  int bet = (game == 3) ? 50000 : 5000;
  if (p->Money() < bet) { ui->Toast("No money for bet"); return; }
  p->AddMoney(-bet);
  int roll = GetRandomValue(0, 100);
  int win = 0;
  if (game == 0) win = (roll > 52) ? bet * 2 : 0;
  else if (game == 1) win = (roll > 48) ? (int)(bet * 1.8f) : 0;
  else if (game == 2) win = (roll > 70) ? bet * 5 : (roll > 55 ? bet : 0);
  else win = (roll > 60) ? bet * 3 : 0;
  p->AddMoney(win);
  if (win) {
    ui->Toast("Casino +" + std::to_string(win));
    if (app_->Cutscenes()) app_->Cutscenes()->Play("casino_bet");
  } else ui->Toast("Casino lose");
}

void GameSystems::BuyWeapon(int index) {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  const char* names[] = {"Knife", "Bat", "Pistol", "SMG", "Shotgun"};
  int prices[] = {5000, 8000, 45000, 120000, 90000};
  if (index < 0 || index > 4) index = 0;
  if (p->Money() < prices[index]) { ui->Toast("Need money"); return; }
  p->AddMoney(-prices[index]);
  p->Inv().weapons.push_back(names[index]);
  p->AddHeat(3);
  ui->Toast(std::string("Bought ") + names[index]);
}

void GameSystems::DoFenceSell() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (p->Inv().cars.empty()) { ui->Toast("No cars to fence"); return; }
  std::string c = p->Inv().cars.back();
  p->Inv().cars.pop_back();
  int pay = 30000 + GetRandomValue(0, 50000);
  p->AddMoney(pay);
  p->AddHeat(12);
  p->SetWanted(std::min(5, p->Wanted() + 1));
  ui->Toast("Fence +" + std::to_string(pay));
}

void GameSystems::DoStimulant() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (p->Money() < 15000) { ui->Toast("Stim $15000"); return; }
  p->AddMoney(-15000);
  p->SetEnergy(100);
  p->AddHeat(8);
  p->SetWanted(std::min(5, p->Wanted() + 1));
  ui->Toast("Stimulant: energy up, wanted +1");
}

void GameSystems::DoBribe() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  int stars = p->Wanted();
  if (stars <= 0) { ui->Toast("No heat"); return; }
  int cost = 15000 * stars;
  if (p->Money() < cost) { ui->Toast("Bribe needs $" + std::to_string(cost)); return; }
  p->AddMoney(-cost);
  if (GetRandomValue(0, 100) < 60 + p->Skill("charisma") * 5) {
    p->SetWanted(0);
    ui->Toast("Bribe success");
  } else {
    p->SetWanted(std::min(5, stars + 1));
    ui->Toast("Bribe failed");
  }
}

void GameSystems::DoSyndicate() {
  auto* p = app_->Player();
  auto* ui = app_->UI();
  if (p->Energy() < 20) { ui->Toast("Need energy"); return; }
  p->SetEnergy(p->Energy() - 20);
  int gain = 40000 + GetRandomValue(0, 60000);
  p->AddMoney(gain);
  p->AddHeat(15);
  p->SetWanted(std::min(5, p->Wanted() + 2));
  ui->Toast("Syndicate point +" + std::to_string(gain));
  if (app_->Cutscenes()) app_->Cutscenes()->Play("syndicate");
}
