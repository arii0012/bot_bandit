#pragma once
#include "raylib.h"
#include <string>
#include <vector>

class Application;

class InventoryUI {
public:
  explicit InventoryUI(Application* app);
  void Toggle();
  bool IsOpen() const { return open_; }
  void Update(float dt);
  void Draw(int sw, int sh);

private:
  Application* app_;
  bool open_ = false;
  int tab_ = 0; // 0 items 1 weapons 2 cars 3 mats 4 clothes
  int selected_ = 0;
  void DrawTab(Rectangle area, int tabId, const char* name);
  void UseSelected();
};
