#pragma once
#include "raylib.h"
#include <string>

class FontUI {
public:
  static FontUI& Get();
  void Init();
  void Shutdown();
  bool Ready() const { return ready_; }

  void Draw(const char* text, float x, float y, float size, Color color);
  void Draw(const std::string& text, float x, float y, float size, Color color);
  void DrawCentered(const char* text, float cx, float y, float size, Color color);
  void DrawShadow(const char* text, float x, float y, float size, Color color, Color shadow = {0, 0, 0, 180});
  void DrawNeon(const char* text, float x, float y, float size, Color color, Color glow = {220, 40, 80, 120});

  int Measure(const char* text, float size);
  int Measure(const std::string& text, float size);
  Font& Face() { return font_; }

private:
  FontUI() = default;
  void BuildPixelFont();
  Font font_{};
  bool ready_ = false;
  bool owned_ = false;
};

inline void DrawTextRu(const char* text, int x, int y, int size, Color color) {
  FontUI::Get().Draw(text, (float)x, (float)y, (float)size, color);
}
inline void DrawTextRu(const std::string& text, int x, int y, int size, Color color) {
  FontUI::Get().Draw(text, (float)x, (float)y, (float)size, color);
}
