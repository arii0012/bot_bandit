#include "FontUI.hpp"
#include <cstring>

FontUI& FontUI::Get() {
  static FontUI inst;
  return inst;
}

void FontUI::Init() {
  if (ready_) return;
  // Cyrillic + Latin + digits + punctuation
  std::vector<int> cps;
  for (int i = 32; i < 127; ++i) cps.push_back(i);
  // Cyrillic block
  for (int i = 0x0400; i <= 0x04FF; ++i) cps.push_back(i);
  cps.push_back(0x2116); // №
  cps.push_back(0x00B0); // °

  const char* candidates[] = {
    "assets/fonts/NotoSans-Regular.ttf",
    "assets/fonts/DejaVuSans.ttf",
    "C:/Windows/Fonts/arial.ttf",
    "C:/Windows/Fonts/segoeui.ttf",
    "C:/Windows/Fonts/tahoma.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    nullptr
  };

  for (int i = 0; candidates[i]; ++i) {
    if (!FileExists(candidates[i])) continue;
    font_ = LoadFontEx(candidates[i], 32, cps.data(), (int)cps.size());
    if (font_.texture.id != 0) {
      SetTextureFilter(font_.texture, TEXTURE_FILTER_BILINEAR);
      ready_ = true;
      TraceLog(LOG_INFO, "FontUI loaded: %s", candidates[i]);
      return;
    }
  }
  // fallback default (no cyrillic)
  font_ = GetFontDefault();
  ready_ = false;
  TraceLog(LOG_WARNING, "FontUI: no TTF with Cyrillic found");
}

void FontUI::Shutdown() {
  if (ready_ && font_.texture.id != 0 && font_.texture.id != GetFontDefault().texture.id)
    UnloadFont(font_);
  ready_ = false;
}

void FontUI::Draw(const char* text, float x, float y, float size, Color color) {
  if (!text) return;
  if (ready_)
    DrawTextEx(font_, text, {x, y}, size, 1.0f, color);
  else
    DrawText(text, (int)x, (int)y, (int)size, color);
}

void FontUI::Draw(const std::string& text, float x, float y, float size, Color color) {
  Draw(text.c_str(), x, y, size, color);
}

int FontUI::Measure(const char* text, float size) {
  if (ready_)
    return (int)MeasureTextEx(font_, text, size, 1.0f).x;
  return MeasureText(text, (int)size);
}
