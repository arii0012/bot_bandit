#pragma once
#include "raylib.h"
#include <string>
#include <vector>

class Application;

class DevConsole {
public:
  explicit DevConsole(Application* app);
  void Update();
  void Draw();

private:
  Application* app_;
  std::string input_;
  std::vector<std::string> history_;
  std::vector<std::string> log_;
  void Execute(const std::string& line);
  void Print(const std::string& s);
};
