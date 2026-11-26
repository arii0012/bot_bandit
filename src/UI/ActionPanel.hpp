#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Character/Character.hpp"
#include "Systems/AntiAbuse.hpp"
#include <string>
#include <cstdio>
#include <algorithm>

enum class ActionId {
  Work = 0,
  Rob,
  Fence,
  LayLow,
  Shop,
  Travel,
  Rest,
  Count
};

struct ActionDef {
  ActionId id;
  const char* label;
  int energyCost;
  int heatGain;
  float cooldownSec;
  int64_t moneyHint; // 0 if variable
};

class ActionPanel {
public:
  void SetHighlight(ActionId id, bool on) {
    highlight_ = on ? id : ActionId::Count;
  }

  ActionId Update(Character& p, float now) {
    ActionId clicked = ActionId::Count;
    int n = static_cast<int>(ActionId::Count);
    for (int i = 0; i < n; ++i) {
      const auto& d = defs_[i];
      bool cd = AntiAbuse::Get().Remaining(CdKey(d.id), d.cooldownSec, now) > 0.01f;
      bool noEn = p.Energy() < d.energyCost;
      Rectangle r = SlotRect(i);
      bool h = CheckCollisionPointRec(GetMousePosition(), r);
      if (h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !cd && !noEn) {
        if (AntiAbuse::Get().TryAction(CdKey(d.id), d.cooldownSec, now)) {
          clicked = d.id;
          p.SetEnergy(p.Energy() - d.energyCost);
          if (d.heatGain > 0) p.AddHeat(d.heatGain);
        }
      }
    }
    return clicked;
  }

  void Draw(Character& p, float now, int sw, int sh) {
    float barH = 72.f;
    float y = static_cast<float>(sh) - barH;
    DrawRectangleRec(Rectangle{0, y - 4, static_cast<float>(sw), barH + 4}, Color{8, 8, 14, 230});
    DrawRectangleRec(Rectangle{0, y - 4, static_cast<float>(sw), 2}, Color{220, 40, 70, 160});

    int n = static_cast<int>(ActionId::Count);
    for (int i = 0; i < n; ++i) {
      const auto& d = defs_[i];
      Rectangle r = SlotRect(i);
      float left = AntiAbuse::Get().Remaining(CdKey(d.id), d.cooldownSec, now);
      bool cd = left > 0.01f;
      bool noEn = p.Energy() < d.energyCost;
      bool h = CheckCollisionPointRec(GetMousePosition(), r);
      bool hi = (highlight_ == d.id);

      Color fill = Color{28, 28, 42, 230};
      if (hi) fill = Color{40, 60, 90, 255};
      if (h && !cd && !noEn) fill = Color{255, 55, 95, 255};
      if (cd || noEn) fill = Color{22, 22, 30, 200};

      DrawRectangleRounded(r, 0.18f, 8, fill);
      if (hi) {
        float pulse = 0.5f + 0.5f * sinf(static_cast<float>(GetTime()) * 5.f);
        DrawRectangleRoundedLines(r, 0.18f, 8, Fade(Color{0, 220, 255, 255}, 0.5f + pulse * 0.5f));
      } else {
        DrawRectangleRoundedLines(r, 0.18f, 8, Color{0, 200, 255, 70});
      }

      FontUI::Get().DrawCentered(d.label, r.x + r.width * 0.5f, r.y + 8.f, 13, WHITE);
      char sub[64];
      if (cd) {
        snprintf(sub, sizeof(sub), "%.0fs", left);
      } else if (noEn) {
        snprintf(sub, sizeof(sub), "EN %d", d.energyCost);
      } else {
        snprintf(sub, sizeof(sub), "EN-%d", d.energyCost);
        if (d.heatGain > 0) {
          char tmp[64];
          snprintf(tmp, sizeof(tmp), " EN-%d H+%d", d.energyCost, d.heatGain);
          snprintf(sub, sizeof(sub), "%s", tmp);
        }
      }
      FontUI::Get().DrawCentered(sub, r.x + r.width * 0.5f, r.y + 28.f, 11,
                                 (cd || noEn) ? Color{160, 80, 80, 255} : Color{160, 200, 180, 255});
      if (d.moneyHint > 0 && !cd) {
        std::string rub = ArtworkRenderer::FormatRub(d.moneyHint);
        FontUI::Get().DrawCentered(rub.c_str(), r.x + r.width * 0.5f, r.y + 46.f, 11, Color{244, 196, 48, 255});
      }
    }

    // resource strip
    char buf[96];
    snprintf(buf, sizeof(buf), "ЭН %d/100   ЖАР %d   %s", p.Energy(), p.Heat(),
             ArtworkRenderer::FormatRub(p.Money()).c_str());
    FontUI::Get().Draw(buf, 12.f, y - 22.f, 14, Color{200, 200, 220, 255});
  }

private:
  static const char* CdKey(ActionId id) {
    switch (id) {
      case ActionId::Work: return "act_work";
      case ActionId::Rob: return "act_rob";
      case ActionId::Fence: return "act_fence";
      case ActionId::LayLow: return "act_laylow";
      case ActionId::Shop: return "act_shop";
      case ActionId::Travel: return "act_travel";
      case ActionId::Rest: return "act_rest";
      default: return "act";
    }
  }

  Rectangle SlotRect(int i) const {
    float margin = 8.f;
    float gap = 6.f;
    float totalW = 1280.f; // will scale in Draw via GetScreenWidth
    totalW = static_cast<float>(GetScreenWidth());
    float w = (totalW - margin * 2 - gap * 6) / 7.f;
    float h = 56.f;
    float x = margin + i * (w + gap);
    float y = static_cast<float>(GetScreenHeight()) - 64.f;
    return Rectangle{x, y, w, h};
  }

  ActionId highlight_ = ActionId::Count;
  ActionDef defs_[7] = {
    {ActionId::Work, "РАБОТА", 12, 0, 14.f, 4000},
    {ActionId::Rob, "ОГРАБЛЕНИЕ", 20, 8, 22.f, 12000},
    {ActionId::Fence, "БАРЫГА", 8, 4, 18.f, 0},
    {ActionId::LayLow, "ЗАЛЕЧЬ", 5, 0, 30.f, -5000},
    {ActionId::Shop, "МАГАЗИН", 0, 0, 1.f, 0},
    {ActionId::Travel, "ПЕРЕЕЗД", 10, 0, 40.f, 0},
    {ActionId::Rest, "ОТДЫХ", 0, 0, 20.f, 0},
  };
};
