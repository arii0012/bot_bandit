#pragma once
#include "raylib.h"

namespace Config {
  inline constexpr int TARGET_FPS = 60;
  inline constexpr float UI_ROUNDNESS = 0.18f;
  inline constexpr float UI_PAD = 12.0f;

  inline Color ColBG()      { return {13, 13, 13, 255}; }
  inline Color ColPanel()   { return {26, 26, 26, 230}; }
  inline Color ColPanel2()  { return {20, 20, 20, 240}; }
  inline Color ColAccent()  { return {230, 57, 70, 255}; }
  inline Color ColGold()    { return {244, 196, 48, 255}; }
  inline Color ColText()    { return {245, 245, 245, 255}; }
  inline Color ColMuted()   { return {138, 138, 138, 255}; }
  inline Color ColSuccess() { return {45, 106, 79, 255}; }
  inline Color ColDanger()  { return {193, 18, 31, 255}; }
  inline Color ColBtn()     { return {47, 47, 47, 255}; }
  inline Color ColBtnHover(){ return {70, 70, 70, 255}; }
}
