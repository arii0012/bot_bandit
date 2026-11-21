#include "ProceduralAssetBuilder.hpp"
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

void ProceduralAssetBuilder::EnsureGenerated(const std::string& assetsRoot) {
  std::string gen = assetsRoot + "/generated";
  fs::create_directories(gen);
  // lightweight: only create if folder empty
  bool empty = true;
  if (fs::exists(gen)) {
    for (auto& _ : fs::directory_iterator(gen)) { empty = false; break; }
  }
  if (!empty) return;

  for (int i = 0; i < 12; ++i) {
    Texture2D t = MakeCharacter(i * 17 + 3, i % 2 == 0);
    Image img = LoadImageFromTexture(t);
    std::string path = gen + "/char_gen_" + std::to_string(i) + ".png";
    ExportImage(img, path.c_str());
    UnloadImage(img);
    UnloadTexture(t);
  }
}

Texture2D ProceduralAssetBuilder::MakeCharacter(int seed, bool female) {
  const int W = 48, H = 64;
  Image img = GenImageColor(W, H, BLANK);
  Color skin = { (unsigned char)(180 + (seed % 40)), (unsigned char)(140 + (seed % 30)), (unsigned char)(120 + (seed % 20)), 255 };
  Color hair = { (unsigned char)(30 + (seed * 3) % 100), (unsigned char)(20 + seed % 50), (unsigned char)(10 + seed % 40), 255 };
  Color shirt = { (unsigned char)(40 + seed % 120), (unsigned char)(40 + (seed * 5) % 100), (unsigned char)(50 + seed % 80), 255 };

  // body
  ImageDrawRectangle(&img, 14, 28, 20, 24, shirt);
  // head
  ImageDrawRectangle(&img, 16, 10, 16, 16, skin);
  // hair
  ImageDrawRectangle(&img, 15, 6, 18, 8, hair);
  if (female) ImageDrawRectangle(&img, 12, 12, 6, 14, hair);
  // legs
  ImageDrawRectangle(&img, 16, 50, 6, 12, Color{30, 30, 40, 255});
  ImageDrawRectangle(&img, 26, 50, 6, 12, Color{30, 30, 40, 255});
  // eyes
  ImageDrawRectangle(&img, 19, 16, 3, 3, BLACK);
  ImageDrawRectangle(&img, 26, 16, 3, 3, BLACK);

  Texture2D t = LoadTextureFromImage(img);
  UnloadImage(img);
  return t;
}

Texture2D ProceduralAssetBuilder::MakeCar(int seed) {
  Image img = GenImageColor(96, 48, BLANK);
  Color body = { (unsigned char)(50 + seed % 150), (unsigned char)(50 + (seed * 3) % 120), (unsigned char)(60 + seed % 100), 255 };
  ImageDrawRectangle(&img, 8, 18, 80, 20, body);
  ImageDrawRectangle(&img, 20, 8, 50, 14, body);
  ImageDrawRectangle(&img, 14, 36, 14, 10, Color{20, 20, 20, 255});
  ImageDrawRectangle(&img, 68, 36, 14, 10, Color{20, 20, 20, 255});
  Texture2D t = LoadTextureFromImage(img);
  UnloadImage(img);
  return t;
}

Texture2D ProceduralAssetBuilder::MakeIcon(int size, Color base) {
  Image img = GenImageColor(size, size, BLANK);
  ImageDrawCircle(&img, size / 2, size / 2, size / 2 - 2, base);
  Texture2D t = LoadTextureFromImage(img);
  UnloadImage(img);
  return t;
}
