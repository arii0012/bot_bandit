#include "raylib.h"
#include "win_admin.hpp"
#include <string>
#include <thread>
#include <atomic>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

static std::string g_dir;
static std::string g_status = "Ready — download from GitHub Releases";
static bool g_shortcut = true;
static bool g_launch = true;
static bool g_busy = false;
static bool g_done = false;
static std::atomic<float> g_progress{0.f};
static std::atomic<bool> g_dlDone{false};
static std::atomic<bool> g_dlOk{false};

static void DrawBtn(Rectangle r, const char* label, bool disabled) {
  bool h = !disabled && CheckCollisionPointRec(GetMousePosition(), r);
  Color c = disabled ? Color{40, 40, 48, 255} : (h ? Color{255, 55, 95, 255} : Color{36, 36, 52, 255});
  DrawRectangleRounded(r, 0.2f, 8, c);
  int tw = MeasureText(label, 18);
  DrawText(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + 12), 18, WHITE);
}

static std::string ReleaseUrl() {
  const char* env = std::getenv("BOTBANDIT_RELEASE_URL");
  if (env && env[0]) return env;
  return WinAdmin::DefaultReleaseZipUrl();
}

static void InstallWorker() {
  g_status = "Connecting to server...";
  g_progress = 0.f;
  g_dlDone = false;
  g_dlOk = false;

  char tmpPath[MAX_PATH] = {};
  GetTempPathA(MAX_PATH, tmpPath);
  std::string zip = std::string(tmpPath) + "BotBandit-Windows-x64.zip";
  WinAdmin::DeleteFileQuiet(zip);

  std::string url = ReleaseUrl();
  std::thread dl([&]() {
    WinAdmin::DownloadFileWithProgress(url, zip, g_progress, g_dlDone, g_dlOk);
  });
  dl.join();

  if (!g_dlOk.load()) {
    g_status = "Download failed — check BOTBANDIT_RELEASE_URL / Releases";
    g_busy = false;
    return;
  }

  g_status = "Extracting resources...";
  g_progress = 0.95f;
  if (!WinAdmin::EnsureDir(g_dir) || !WinAdmin::ExtractZipToFolder(zip, g_dir)) {
    g_status = "Extract failed";
    g_busy = false;
    WinAdmin::DeleteFileQuiet(zip);
    return;
  }
  WinAdmin::DeleteFileQuiet(zip);

  std::string exe = g_dir + "\\botbandit.exe";
  if (g_shortcut) WinAdmin::CreateDesktopShortcut(exe, "BotBandit 3D");
  g_status = "Done!";
  g_progress = 1.f;
  g_done = true;
  g_busy = false;
  if (g_launch) WinAdmin::LaunchExe(exe);
}

int main() {
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(720, 460, "BotBandit Setup");
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);
  g_dir = WinAdmin::DefaultInstallDir();

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(Color{12, 12, 20, 255});
    DrawRectangleRounded(Rectangle{16, 16, 688, 428}, 0.04f, 12, Color{18, 18, 28, 255});

    DrawText("BotBandit 3D — Online Setup", 36, 36, 26, WHITE);
    DrawText("Downloads game zip from GitHub Releases (no admin)", 36, 72, 15, Color{160, 160, 180, 255});

    DrawText("Install path:", 36, 110, 16, Color{180, 180, 200, 255});
    DrawText(g_dir.c_str(), 36, 134, 16, Color{0, 220, 255, 255});
    Rectangle rBrowse{520, 126, 150, 32};
    DrawBtn(rBrowse, "Browse...", g_busy);
    if (!g_busy && CheckCollisionPointRec(GetMousePosition(), rBrowse) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      std::string picked;
      if (WinAdmin::BrowseForFolder(picked)) g_dir = picked + "\\BotBandit";
    }

    Rectangle rShort{36, 180, 26, 26};
    Rectangle rLaunch{36, 216, 26, 26};
    if (!g_busy && CheckCollisionPointRec(GetMousePosition(), rShort) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      g_shortcut = !g_shortcut;
    if (!g_busy && CheckCollisionPointRec(GetMousePosition(), rLaunch) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      g_launch = !g_launch;
    DrawRectangleRec(rShort, g_shortcut ? Color{40, 160, 90, 255} : Color{40, 40, 50, 255});
    DrawRectangleRec(rLaunch, g_launch ? Color{40, 160, 90, 255} : Color{40, 40, 50, 255});
    DrawText("Desktop shortcut", 72, 182, 18, WHITE);
    DrawText("Launch after install", 72, 218, 18, WHITE);

    // progress
    DrawRectangleRounded(Rectangle{36, 270, 648, 22}, 0.3f, 8, Color{30, 30, 40, 255});
    float pw = 648.f * g_progress.load();
    if (pw > 0) DrawRectangleRounded(Rectangle{36, 270, pw, 22}, 0.3f, 8, Color{0, 180, 255, 255});
    char pct[32];
    snprintf(pct, sizeof(pct), "%.0f%%", g_progress.load() * 100.f);
    DrawText(pct, 36, 300, 16, Color{160, 200, 220, 255});
    DrawText(g_status.c_str(), 100, 300, 16, Color{100, 255, 140, 255});

    Rectangle rInst{36, 340, 220, 48};
    DrawBtn(rInst, g_done ? "Installed" : (g_busy ? "Working..." : "Install"), g_busy || g_done);
    if (!g_busy && !g_done && CheckCollisionPointRec(GetMousePosition(), rInst) &&
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      g_busy = true;
      g_status = "Starting download...";
      std::thread(InstallWorker).detach();
    }

    DrawText("ESC - exit", 36, 410, 14, Color{120, 120, 140, 255});
    if (IsKeyPressed(KEY_ESCAPE) && !g_busy) break;
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
