#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>
#include <cstdio>

class TextureAtlasManager {
public:
  static TextureAtlasManager& Get() {
    static TextureAtlasManager inst;
    return inst;
  }

  void Init() {
    if (ready_) return;
    brick_ = MakeBrick(128, 128);
    asphalt_ = MakeAsphalt(128, 128);
    fabric_ = MakeFabric(64, 64);
    metal_ = MakeMetal(64, 64);
    GenTextureMipmaps(&brick_);
    GenTextureMipmaps(&asphalt_);
    GenTextureMipmaps(&fabric_);
    GenTextureMipmaps(&metal_);
    SetTextureFilter(brick_, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(asphalt_, TEXTURE_FILTER_BILINEAR);
    ready_ = true;
  }

  void Shutdown() {
    if (!ready_) return;
    UnloadTexture(brick_);
    UnloadTexture(asphalt_);
    UnloadTexture(fabric_);
    UnloadTexture(metal_);
    for (auto& [_, t] : cache_) UnloadTexture(t);
    cache_.clear();
    ready_ = false;
  }

  Texture2D& Brick() { return brick_; }
  Texture2D& Asphalt() { return asphalt_; }
  Texture2D& Fabric() { return fabric_; }
  Texture2D& Metal() { return metal_; }

private:
  static Texture2D MakeBrick(int w, int h) {
    Image img = GenImageColor(w, h, Color{90, 50, 40, 255});
    for (int y = 0; y < h; y += 8)
      for (int x = 0; x < w; x += 16) {
        int ox = (y / 8) % 2 ? 8 : 0;
        ImageDrawRectangle(&img, x + ox + 1, y + 1, 14, 6, Color{120, 65, 50, 255});
      }
    Texture2D t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
  }

  static Texture2D MakeAsphalt(int w, int h) {
    Image img = GenImageColor(w, h, Color{35, 35, 40, 255});
    for (int i = 0; i < 400; ++i) {
      int x = GetRandomValue(0, w - 1), y = GetRandomValue(0, h - 1);
      ImageDrawPixel(&img, x, y, Color{(unsigned char)GetRandomValue(25, 55),
                                       (unsigned char)GetRandomValue(25, 55),
                                       (unsigned char)GetRandomValue(30, 60), 255});
    }
    Texture2D t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
  }

  static Texture2D MakeFabric(int w, int h) {
    Image img = GenImageColor(w, h, Color{40, 42, 70, 255});
    for (int y = 0; y < h; y += 2)
      ImageDrawLine(&img, 0, y, w, y, Color{50, 52, 80, 255});
    Texture2D t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
  }

  static Texture2D MakeMetal(int w, int h) {
    Image img = GenImageGradientLinear(w, h, 0, Color{80, 80, 90, 255}, Color{40, 40, 48, 255});
    Texture2D t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
  }

  Texture2D brick_{}, asphalt_{}, fabric_{}, metal_{};
  std::unordered_map<std::string, Texture2D> cache_;
  bool ready_ = false;
};
