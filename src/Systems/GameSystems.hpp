#pragma once
#include <string>

class Application;

class GameSystems {
public:
  explicit GameSystems(Application* app);
  void Update(float dt);

  void DoWork(int jobIndex);
  void DoRob();
  void DoHeist();
  void OpenTravel();
  void OpenShop();
  void DoScavenge();
  void DoDaily();
  void DoHide();
  void DoCraft(const std::string& recipeId);
  void OpenCraft();
  void OpenGarage();
  void OpenBusiness();
  void PlayCutscene(const std::string& id);

private:
  Application* app_;
  float workCd_ = 0, robCd_ = 0, heistCd_ = 0, dailyCd_ = 0;
};
