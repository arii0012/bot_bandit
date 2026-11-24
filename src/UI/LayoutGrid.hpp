#pragma once
#include "raylib.h"
#include <algorithm>
#include <cmath>

enum class Anchor {
  TopLeft, TopCenter, TopRight,
  CenterLeft, Center, CenterRight,
  BottomLeft, BottomCenter, BottomRight
};

class LayoutGrid {
public:
  void SetScreen(int w, int h) {
    sw_ = std::max(1, w);
    sh_ = std::max(1, h);
    scale_ = std::min(sw_ / refW_, sh_ / refH_);
    if (scale_ < 0.5f) scale_ = 0.5f;
    if (scale_ > 2.5f) scale_ = 2.5f;
  }

  float Scale() const { return scale_; }
  int SW() const { return sw_; }
  int SH() const { return sh_; }

  // design-space rect -> screen rect
  Rectangle Place(float x, float y, float w, float h, Anchor a = Anchor::TopLeft) const {
    float sx = x * scale_;
    float sy = y * scale_;
    float sw = w * scale_;
    float sh = h * scale_;
    switch (a) {
      case Anchor::TopLeft: break;
      case Anchor::TopCenter: sx = sw_ * 0.5f + x * scale_ - sw * 0.5f; break;
      case Anchor::TopRight: sx = sw_ - x * scale_ - sw; break;
      case Anchor::CenterLeft: sy = sh_ * 0.5f + y * scale_ - sh * 0.5f; break;
      case Anchor::Center:
        sx = sw_ * 0.5f + x * scale_ - sw * 0.5f;
        sy = sh_ * 0.5f + y * scale_ - sh * 0.5f;
        break;
      case Anchor::CenterRight:
        sx = sw_ - x * scale_ - sw;
        sy = sh_ * 0.5f + y * scale_ - sh * 0.5f;
        break;
      case Anchor::BottomLeft: sy = sh_ - y * scale_ - sh; break;
      case Anchor::BottomCenter:
        sx = sw_ * 0.5f + x * scale_ - sw * 0.5f;
        sy = sh_ - y * scale_ - sh;
        break;
      case Anchor::BottomRight:
        sx = sw_ - x * scale_ - sw;
        sy = sh_ - y * scale_ - sh;
        break;
    }
    if (sx < 4) sx = 4;
    if (sy < 4) sy = 4;
    if (sx + sw > sw_ - 4) sw = std::max(8.f, sw_ - 4 - sx);
    if (sy + sh > sh_ - 4) sh = std::max(8.f, sh_ - 4 - sy);
    return Rectangle{sx, sy, sw, sh};
  }

  float Font(float designSize) const {
    float s = designSize * scale_;
    if (s < 12.f) s = 12.f;
    if (s > 64.f) s = 64.f;
    return s;
  }

  // scrollable list helper
  struct ScrollState {
    float offset = 0.f;
    float contentH = 0.f;
    float viewH = 0.f;
  };

  void HandleScroll(ScrollState& s, Rectangle view) const {
    float wheel = GetMouseWheelMove();
    if (CheckCollisionPointRec(GetMousePosition(), view) && wheel != 0.f) {
      s.offset -= wheel * 40.f * scale_;
    }
    float maxOff = std::max(0.f, s.contentH - s.viewH);
    if (s.offset < 0) s.offset = 0;
    if (s.offset > maxOff) s.offset = maxOff;
  }

  static void DrawScrollBar(Rectangle view, const ScrollState& s) {
    if (s.contentH <= s.viewH + 1.f) return;
    float ratio = s.viewH / s.contentH;
    float barH = view.height * ratio;
    float t = (s.contentH > s.viewH) ? (s.offset / (s.contentH - s.viewH)) : 0.f;
    float barY = view.y + t * (view.height - barH);
    DrawRectangleRounded(Rectangle{view.x + view.width - 8, view.y, 6, view.height}, 0.5f, 4, Color{30, 30, 40, 180});
    DrawRectangleRounded(Rectangle{view.x + view.width - 8, barY, 6, barH}, 0.5f, 4, Color{220, 40, 70, 200});
  }

private:
  int sw_ = 1280, sh_ = 720;
  float scale_ = 1.f;
  static constexpr float refW_ = 1920.f;
  static constexpr float refH_ = 1080.f;
};
