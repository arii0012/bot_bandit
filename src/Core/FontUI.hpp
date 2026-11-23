#pragma once
#include "raylib.h"
#include <string>
#include <vector>

class FontUI {
public:
  static FontUI& Get();
  void Init();
  void Shutdown();
  void Draw(const char* text, float x, float y, float size, Color color);
  void Draw(const std::string& text, float x, float y, float size, Color color);
  int Measure(const char* text, float size);
  Font& Face() { return font_; }
  bool Ready() const { return ready_; }

private:
  FontUI() = default;
  Font font_{};
  bool ready_ = false;
};

// Drop-in helpers used across codebase
inline void DrawTextRu(const char* text, int x, int y, int size, Color color) {
  FontUI::Get().Draw(text, (float)x, (float)y, (float)size, color);
}
inline void DrawTextRu(const std::string& text, int x, int y, int size, Color color) {
  FontUI::Get().Draw(text, (float)x, (float)y, (float)size, color);
}
