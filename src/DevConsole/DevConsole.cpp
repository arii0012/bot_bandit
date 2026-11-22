#include "DevConsole.hpp"
#include "Core/Application.hpp"
#include "Core/Config.hpp"
#include "Character/Character.hpp"
#include "World/World.hpp"
#include "World/City.hpp"
#include "Systems/Weather.hpp"
#include "Systems/GameSystems.hpp"
#include <sstream>
#include <algorithm>

DevConsole::DevConsole(Application* app) : app_(app) {
  log_.push_back("BOT BANDIT console. Type /help");
}

void DevConsole::Print(const std::string& s) {
  log_.push_back(s);
  if (log_.size() > 40) log_.erase(log_.begin());
}

void DevConsole::Update() {
  int ch = GetCharPressed();
  while (ch > 0) {
    if (ch >= 32 && ch < 127) input_ += (char)ch;
    ch = GetCharPressed();
  }
  if (IsKeyPressed(KEY_BACKSPACE) && !input_.empty()) input_.pop_back();
  if (IsKeyPressed(KEY_ENTER) && !input_.empty()) {
    history_.push_back(input_);
    Execute(input_);
    input_.clear();
  }
}

void DevConsole::Execute(const std::string& line) {
  Print("> " + line);
  std::istringstream iss(line);
  std::string cmd;
  iss >> cmd;
  std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

  auto* p = app_->Player();
  auto* w = app_->GetWorld();

  if (cmd == "/help") {
    Print("/give_money N  /wanted N  /tp city  /godmode");
    Print("/spawn_car name  /unlock_all  /max_stats  /name X");
    Print("/exit  /save  /weather rain|clear|neon|fog");
    Print("/garage  /biz  /cutscene id");
  } else if (cmd == "/give_money") {
    int64_t v = 0; iss >> v;
    p->AddMoney(v);
    Print("money now " + std::to_string(p->Money()));
  } else if (cmd == "/wanted") {
    int v = 0; iss >> v;
    p->SetWanted(v);
    Print("wanted " + std::to_string(p->Wanted()));
  } else if (cmd == "/tp") {
    std::string id; iss >> id;
    if (FindCity(id)) {
      w->SetCity(id); p->SetCity(id);
      Print("tp " + id);
    } else Print("unknown city");
  } else if (cmd == "/godmode") {
    p->SetEnergy(100);
    p->SetWanted(0);
    p->AddHeat(-100);
    Print("godmode pulse");
  } else if (cmd == "/max_stats") {
    p->strength_ = 50; p->agility_ = 50; p->intelligence_ = 50;
    p->level_ = 20; p->AddSkillPoints(10);
    Print("stats maxed");
  } else if (cmd == "/unlock_all") {
    p->AddMoney(5000000);
    p->Inv().cars.push_back("Lamborghini");
    p->Inv().cars.push_back("Tesla");
    p->SetActiveCar("Lamborghini");
    Print("unlocked");
  } else if (cmd == "/spawn_car") {
    std::string name; iss >> name;
    if (name.empty()) name = "lambo";
    p->Inv().cars.push_back(name);
    p->SetActiveCar(name);
    Print("spawned " + name);
  } else if (cmd == "/name") {
    std::string n; std::getline(iss, n);
    if (!n.empty() && n[0] == ' ') n.erase(0, 1);
    if (!n.empty()) { p->SetName(n); Print("name set"); }
  } else if (cmd == "/save") {
    app_->Saves()->Save(*p, *w);
    Print("saved");
  
  } else if (cmd == "/weather") {
    std::string w; iss >> w;
    if (!app_->Weather()) { Print("no weather"); }
    else if (w == "rain") { app_->Weather()->Set(WeatherType::Rain); Print("rain"); }
    else if (w == "clear") { app_->Weather()->Set(WeatherType::Clear); Print("clear"); }
    else if (w == "neon") { app_->Weather()->Set(WeatherType::NeonNight); Print("neon"); }
    else if (w == "fog") { app_->Weather()->Set(WeatherType::Fog); Print("fog"); }
    else Print("rain|clear|neon|fog");
  } else if (cmd == "/cutscene") {
    std::string id; iss >> id;
    if (id.empty()) id = "debut";
    if (app_->Cutscenes()) app_->Cutscenes()->Play(id);
    Print("play " + id);
  } else if (cmd == "/garage") {
    app_->Systems()->OpenGarage();
  } else if (cmd == "/biz") {
    app_->Systems()->OpenBusiness();

  } else if (cmd == "/fight") {
    app_->Systems()->DoFight();
  } else if (cmd == "/pet") {
    app_->Systems()->BuyPet();
  } else if (cmd == "/smuggle") {
    app_->Systems()->DoSmuggle();
  } else if (cmd == "/race") {
    app_->Systems()->DoRace();
  } else if (cmd == "/home") {
    app_->Systems()->OpenSmartHome();
  } else if (cmd == "/studio") {
    app_->Systems()->OpenStudio();
  } else if (cmd == "/autopilot") {
    app_->Systems()->DoAutopilotTravel();
  } else if (cmd == "/exit") {
    app_->RequestExit();
  } else {
    Print("unknown. /help");
  }
}

void DevConsole::Draw() {
  int sw = app_->ScreenW();
  float h = 280;
  DrawRectangle(0, 0, sw, (int)h, Fade(BLACK, 0.85f));
  DrawRectangleLines(0, 0, sw, (int)h, Config::ColAccent());

  int y = 8;
  for (size_t i = 0; i < log_.size(); ++i) {
    DrawText(log_[i].c_str(), 12, y, 16, Config::ColText());
    y += 18;
  }
  DrawText(("> " + input_ + "_").c_str(), 12, (int)h - 28, 18, Config::ColGold());
}
