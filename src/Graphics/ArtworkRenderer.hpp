#pragma once
#include "raylib.h"
#include <string>
#include <cstdio>
#include <cstdint>

class ArtworkRenderer {
public:
  // 120000 -> "120 000 P" (ASCII P used as ruble stand-in for pixel font)
  static std::string FormatRub(int64_t amount) {
    bool neg = amount < 0;
    if (neg) amount = -amount;
    char digits[32];
    snprintf(digits, sizeof(digits), "%lld", (long long)amount);
    std::string raw(digits);
    std::string out;
    int n = (int)raw.size();
    for (int i = 0; i < n; ++i) {
      if (i > 0 && (n - i) % 3 == 0) out.push_back(' ');
      out.push_back(raw[i]);
    }
    if (neg) out.insert(out.begin(), '-');
    out += " P";
    return out;
  }

  static Texture2D MakeGradientCover(int w, int h, Color top, Color bot, const char* title) {
    Image img = GenImageColor(w, h, bot);
    for (int y = 0; y < h; ++y) {
      float t = (float)y / (float)(h > 1 ? h - 1 : 1);
      Color c = {
        (unsigned char)(top.r + (bot.r - top.r) * t),
        (unsigned char)(top.g + (bot.g - top.g) * t),
        (unsigned char)(top.b + (bot.b - top.b) * t),
        255};
      ImageDrawLine(&img, 0, y, w, y, c);
    }
    // neon bars
    ImageDrawRectangle(&img, 0, h / 3, w, 3, Color{220, 40, 80, 200});
    ImageDrawRectangle(&img, 0, h / 3 + 8, w, 1, Color{0, 200, 255, 160});
    // city blocks silhouette
    for (int i = 0; i < 12; ++i) {
      int bx = 10 + i * (w / 12);
      int bh = 20 + (i * 37) % (h / 3);
      ImageDrawRectangle(&img, bx, h - bh - 10, w / 16, bh, Color{10, 10, 18, 220});
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    (void)title;
    return tex;
  }

  static Texture2D MakeCarCard(int w, int h, Color body) {
    Image img = GenImageColor(w, h, Color{16, 16, 24, 255});
    int cx = w / 2, cy = h / 2 + 8;
    ImageDrawRectangle(&img, cx - 40, cy - 12, 80, 24, body);
    ImageDrawRectangle(&img, cx - 28, cy - 24, 50, 16, Color{
      (unsigned char)(body.r * 0.7f), (unsigned char)(body.g * 0.7f),
      (unsigned char)(body.b * 0.7f), 255});
    ImageDrawCircle(&img, cx - 28, cy + 12, 8, Color{20, 20, 25, 255});
    ImageDrawCircle(&img, cx + 28, cy + 12, 8, Color{20, 20, 25, 255});
    ImageDrawRectangle(&img, cx + 32, cy - 6, 8, 5, Color{255, 230, 150, 255});
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
  }

  static Texture2D MakeHouseCard(int w, int h, int tiers) {
    Image img = GenImageColor(w, h, Color{12, 14, 22, 255});
    int bh = 30 + tiers * 18;
    int bx = w / 2 - 28, by = h - 20 - bh;
    ImageDrawRectangle(&img, bx, by, 56, bh, Color{40, 42, 55, 255});
    for (int r = 0; r < tiers * 2; ++r)
      for (int c = 0; c < 3; ++c)
        ImageDrawRectangle(&img, bx + 6 + c * 16, by + 8 + r * 12, 10, 8, Color{255, 210, 120, 200});
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
  }

  static Texture2D MakeCityPostcard(int w, int h, int citySeed) {
    Color tops[] = {
      {30, 40, 80, 255}, {60, 30, 50, 255}, {20, 60, 90, 255},
      {40, 80, 100, 255}, {50, 30, 70, 255}};
    Color bots[] = {
      {180, 100, 60, 255}, {20, 20, 35, 255}, {100, 160, 200, 255},
      {240, 200, 120, 255}, {90, 40, 80, 255}};
    int i = citySeed % 5;
    return MakeGradientCover(w, h, tops[i], bots[i], nullptr);
  }

  static Texture2D MakeChapterArt(int chapterIndex) {
    Color a = {(unsigned char)(20 + chapterIndex * 15), 20,
               (unsigned char)(40 + chapterIndex * 10), 255};
    Color b = {(unsigned char)(180 - chapterIndex * 8), 40, 70, 255};
    return MakeGradientCover(640, 360, a, b, nullptr);
  }

  static Texture2D MakeAvatar(bool female, Color hair) {
    Image img = GenImageColor(64, 64, Color{24, 24, 36, 255});
    Color skin = female ? Color{232, 186, 162, 255} : Color{210, 168, 140, 255};
    ImageDrawCircle(&img, 32, 28, 18, skin);
    ImageDrawCircle(&img, 32, 18, 16, hair);
    ImageDrawCircle(&img, 26, 28, 3, WHITE);
    ImageDrawCircle(&img, 38, 28, 3, WHITE);
    ImageDrawCircle(&img, 26, 28, 1, BLACK);
    ImageDrawCircle(&img, 38, 28, 1, BLACK);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
  }
};
