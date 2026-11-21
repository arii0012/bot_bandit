#pragma once
#include "raylib.h"
#include <string>

class ProceduralAssetBuilder {
public:
  // Generate missing HD-style pixel art to disk under assets/generated/
  static void EnsureGenerated(const std::string& assetsRoot);
  static Texture2D MakeCharacter(int seed, bool female);
  static Texture2D MakeCar(int seed);
  static Texture2D MakeIcon(int size, Color base);
};
