#include "raylib.h"
#include "win_admin.hpp"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <string>
#include <cstdio>

static bool g_shortcut = true;
static bool g_launch = true;
static std::string g_dir;
static std::string g_status = "Ready";
static bool g_done = false;

static void DrawBtn(Rectangle r, const char* label, bool on) {
  bool h = CheckCollisionPointRec(GetMousePosition(), r);
  DrawRectangleRounded(r, 0.2f, 8, on ? Color{40, 160, 90, 255} : (h ? Color{255, 55, 95, 255} : Color{36, 36, 52, 255}));
  int tw = MeasureText(label, 18);
  DrawText(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + 10), 18, WHITE);
}

int main() {
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(640, 420, "BotBandit Setup");
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  g_dir = WinAdmin::DefaultInstallDir();

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(Color{12, 12, 20, 255});
    DrawText("BotBandit 3D Setup", 24, 20, 28, WHITE);
    DrawText("Install folder (no admin required):", 24, 70, 16, Color{180, 180, 200, 255});
    DrawText(g_dir.c_str(), 24, 96, 16, Color{0, 220, 255, 255});

    Rectangle rShort{24, 140, 28, 28};
    Rectangle rLaunch{24, 180, 28, 28};
    bool hS = CheckCollisionPointRec(GetMousePosition(), rShort);
    bool hL = CheckCollisionPointRec(GetMousePosition(), rLaunch);
    if (hS && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) g_shortcut = !g_shortcut;
    if (hL && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) g_launch = !g_launch;
    DrawRectangleRec(rShort, g_shortcut ? Color{40, 160, 90, 255} : Color{40, 40, 50, 255});
    DrawRectangleRec(rLaunch, g_launch ? Color{40, 160, 90, 255} : Color{40, 40, 50, 255});
    DrawText("Create desktop shortcut", 60, 144, 18, WHITE);
    DrawText("Launch after install", 60, 184, 18, WHITE);

    Rectangle rInstall{24, 240, 200, 44};
    DrawBtn(rInstall, g_done ? "Done" : "Install", false);
    if (!g_done && CheckCollisionPointRec(GetMousePosition(), rInstall) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      g_status = "Installing...";
      if (!WinAdmin::EnsureDir(g_dir)) {
        g_status = "Cannot create folder";
      } else {
        // copy next to setup.exe
        char mod[MAX_PATH] = {};
        GetModuleFileNameA(nullptr, mod, MAX_PATH);
        std::string setupDir = mod;
        size_t slash = setupDir.find_last_of("\\/");
        if (slash != std::string::npos) setupDir.resize(slash);
        WinAdmin::CopyTree(setupDir, g_dir);
        std::string exe = g_dir + "\\botbandit.exe";
        if (g_shortcut) WinAdmin::CreateDesktopShortcut(exe, "BotBandit 3D");
        g_status = "Installed to LOCALAPPDATA\\BotBandit";
        g_done = true;
        if (g_launch) WinAdmin::LaunchExe(exe);
      }
    }

    DrawText(g_status.c_str(), 24, 310, 18, Color{100, 255, 140, 255});
    DrawText("ESC - exit", 24, 380, 14, Color{140, 140, 160, 255});
    if (IsKeyPressed(KEY_ESCAPE)) break;
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
