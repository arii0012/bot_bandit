#pragma once
#include "raylib.h"
#include "Core/FontUI.hpp"
#include "Graphics/ArtworkRenderer.hpp"
#include "Character/Character.hpp"
#include <cstdlib>
#include <cmath>
#include <string>

class CardBluffMinigame {
public:
  void Open(int64_t bet) {
    active_ = true;
    bet_ = bet;
    playerScore_ = GetRandomValue(12, 20);
    npcScore_ = GetRandomValue(12, 21);
    npcSweat_ = (float)GetRandomValue(0, 100) / 100.f;
    npcDelay_ = 0.4f + (float)GetRandomValue(0, 80) / 100.f;
    stress_ = 0.2f;
    done_ = false;
    win_ = false;
    t_ = 0.f;
  }

  void Close() { active_ = false; }
  bool Active() const { return active_; }
  bool JustWon() const { return done_ && win_; }
  int64_t Bet() const { return bet_; }

  void Update(float dt, Character& p) {
    if (!active_ || done_) return;
    t_ += dt;
    stress_ = std::min(1.f, stress_ + dt * 0.05f);
    if (IsKeyPressed(KEY_R)) {
      // raise / bluff
      stress_ += 0.15f;
      npcSweat_ += 0.1f;
      if (GetRandomValue(0, 100) < 40 + (int)(npcSweat_ * 30)) {
        // npc folds
        done_ = true;
        win_ = true;
        p.AddMoney(bet_);
      }
    }
    if (IsKeyPressed(KEY_SPACE) || t_ > 3.f + npcDelay_) {
      done_ = true;
      win_ = playerScore_ >= npcScore_ && playerScore_ <= 21;
      if (playerScore_ > 21) win_ = false;
      if (win_) p.AddMoney(bet_);
      else p.AddMoney(-bet_);
    }
    if (IsKeyPressed(KEY_ESCAPE)) { Close(); }
  }

  void Draw(int sw, int sh) {
    if (!active_) return;
    DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 180});
    float x = sw * 0.5f - 260.f, y = sh * 0.5f - 150.f;
    DrawRectangleRounded(Rectangle{x, y, 520, 300}, 0.08f, 12, Color{20, 20, 32, 245});
    FontUI::Get().DrawCentered("VIP STOL - BLEF", sw * 0.5f, y + 16, 22, Color{244, 196, 48, 255});
    char buf[64];
    snprintf(buf, sizeof(buf), "Ty: %d   Sopernik: %s", playerScore_, done_ ? std::to_string(npcScore_).c_str() : "??");
    FontUI::Get().DrawCentered(buf, sw * 0.5f, y + 60, 20, WHITE);
    snprintf(buf, sizeof(buf), "Stavka %s", ArtworkRenderer::FormatRub(bet_).c_str());
    FontUI::Get().DrawCentered(buf, sw * 0.5f, y + 95, 16, Color{0, 220, 255, 255});
    // tells
    FontUI::Get().Draw("Telly NPC:", x + 24, y + 130, 14, Color{160, 160, 180, 255});
    DrawRectangleRec(Rectangle{x + 24, y + 155, 200 * npcSweat_, 10}, Color{255, 100, 80, 255});
    FontUI::Get().Draw("Pot / zaderzhka", x + 24, y + 170, 12, Color{140, 140, 160, 255});
    DrawRectangleRec(Rectangle{x + 24, y + 200, 200 * stress_, 10}, Color{255, 200, 60, 255});
    FontUI::Get().Draw("Tvoy stress", x + 24, y + 215, 12, Color{140, 140, 160, 255});
    FontUI::Get().DrawCentered("R - reyz/blef   SPACE - vskryt'   ESC - uytі", sw * 0.5f, y + 260, 14,
                               Color{180, 180, 200, 255});
    if (done_) {
      FontUI::Get().DrawCentered(win_ ? "VYIGRYSH" : "PROIGRYSH", sw * 0.5f, y + 110, 24,
                                 win_ ? Color{100, 255, 140, 255} : Color{255, 80, 80, 255});
    }
  }

private:
  bool active_ = false, done_ = false, win_ = false;
  int64_t bet_ = 0;
  int playerScore_ = 0, npcScore_ = 0;
  float npcSweat_ = 0, npcDelay_ = 0, stress_ = 0, t_ = 0;
};
