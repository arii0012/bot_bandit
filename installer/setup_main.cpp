#include "raylib.h"
#include "win_admin.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cstdio>

namespace fs = std::filesystem;

static Color CBg() { return {10, 10, 16, 255}; }
static Color CPanel() { return {22, 22, 32, 230}; }
static Color CAcc() { return {220, 40, 70, 255}; }
static Color CBtn() { return {40, 40, 55, 255}; }
static Color CBtnH() { return {60, 50, 75, 255}; }
static Color CTxt() { return {240, 240, 245, 255}; }
static Color CMut() { return {140, 140, 155, 255}; }
static Color COk() { return {40, 180, 100, 255}; }

static bool Btn(Rectangle r, const char* label, bool primary) {
  Vector2 m = GetMousePosition();
  bool h = CheckCollisionPointRec(m, r);
  DrawRectangleRounded(r, 0.25f, 8, primary ? (h ? Color{255, 60, 90, 255} : CAcc()) : (h ? CBtnH() : CBtn()));
  DrawRectangleRoundedLines(r, 0.25f, 8, h ? WHITE : Color{80, 80, 100, 255});
  int tw = MeasureText(label, 20);
  DrawText(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + (r.height - 20) * 0.5f), 20, CTxt());
  return h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void Checkbox(Rectangle r, const char* label, bool* on) {
  Vector2 m = GetMousePosition();
  bool h = CheckCollisionPointRec(m, r);
  DrawRectangleRounded(Rectangle{r.x, r.y, 22, 22}, 0.2f, 4, h ? CBtnH() : CBtn());
  DrawRectangleRoundedLines(Rectangle{r.x, r.y, 22, 22}, 0.2f, 4, *on ? COk() : Color{80, 80, 100, 255});
  if (*on) {
    DrawLineEx(Vector2{r.x + 5, r.y + 11}, Vector2{r.x + 9, r.y + 16}, 2.5f, COk());
    DrawLineEx(Vector2{r.x + 9, r.y + 16}, Vector2{r.x + 17, r.y + 6}, 2.5f, COk());
  }
  DrawText(label, (int)(r.x + 30), (int)(r.y + 3), 18, CTxt());
  if (h && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) *on = !*on;
}

static std::string DefaultDir() {
#if defined(_WIN32)
  return "C:\\Games\\BotBandit";
#else
  const char* h = getenv("HOME");
  return std::string(h ? h : ".") + "/Games/BotBandit";
#endif
}

static void Collect(const fs::path& root, std::vector<fs::path>& out) {
  std::error_code ec;
  if (!fs::exists(root, ec)) return;
  for (auto it = fs::recursive_directory_iterator(root, ec); it != fs::recursive_directory_iterator(); it.increment(ec)) {
    if (ec) break;
    if (it->is_regular_file(ec)) out.push_back(it->path());
  }
}

enum class Phase { Form, Installing, Done, Fail };

int main() {
  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  const int W = 780, H = 520;
  InitWindow(W, H, "LifeSim3D Setup");
  SetTargetFPS(60);
  HideCursor();

  std::string pathEdit = DefaultDir();
  bool makeShortcut = true, launchAfter = true;
  Phase phase = Phase::Form;
  std::string status = IsRunningAsAdmin() ? "Running as Administrator" : "User mode (OK for C:\\Games)";
  std::string lastError;
  float progress = 0.f;
  std::vector<fs::path> queue;
  size_t qi = 0;
  bool editing = false;

  fs::path selfDir = GetModuleDirectory();

  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    BeginDrawing();
    ClearBackground(CBg());
    DrawRectangleRounded(Rectangle{24, 24, (float)W - 48.f, (float)H - 48.f}, 0.06f, 12, CPanel());
    DrawRectangleRoundedLines(Rectangle{24, 24, (float)W - 48.f, (float)H - 48.f}, 0.06f, 12, CAcc());
    DrawText("LifeSim3D / BOT BANDIT", 48, 44, 28, CTxt());
    DrawText("UNDERWORLD  Setup", 48, 78, 18, CMut());

    if (phase == Phase::Form) {
      DrawText("Install folder:", 48, 130, 18, CMut());
      Rectangle pathBox{48, 158, 520, 40};
      DrawRectangleRounded(pathBox, 0.15f, 6, Color{16, 16, 24, 255});
      DrawRectangleRoundedLines(pathBox, 0.15f, 6, editing ? CAcc() : Color{70, 70, 90, 255});
      if (CheckCollisionPointRec(mouse, pathBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) editing = true;
      if (editing) {
        int ch = GetCharPressed();
        while (ch > 0) {
          if (ch >= 32 && ch < 127 && pathEdit.size() < 240) pathEdit.push_back((char)ch);
          ch = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !pathEdit.empty()) pathEdit.pop_back();
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) editing = false;
      }
      DrawText(pathEdit.c_str(), 58, 168, 18, CTxt());
      if (Btn(Rectangle{580, 158, 140, 40}, "Browse...", false)) {
        editing = false;
        std::string p = BrowseFolderDialog();
        if (!p.empty()) pathEdit = p + "\\BotBandit";
      }
      Checkbox(Rectangle{48, 220, 400, 28}, "Create desktop shortcut", &makeShortcut);
      Checkbox(Rectangle{48, 260, 400, 28}, "Launch game after install", &launchAfter);
      DrawText(status.c_str(), 48, 320, 16, CMut());
      if (Btn(Rectangle{48, 400, 200, 48}, "Install", true)) {
        if (NeedsAdminPath(pathEdit) && !IsRunningAsAdmin()) {
          status = "Requesting Administrator (UAC)...";
          if (RelaunchAsAdmin()) break;
          status = "UAC denied — choose C:\\Games\\BotBandit";
        } else {
          queue.clear();
          qi = 0;
          for (const char* n : {"botbandit.exe", "botbandit"}) {
            fs::path p = selfDir / n;
            if (fs::exists(p)) queue.push_back(p);
          }
          Collect(selfDir / "assets", queue);
          Collect(selfDir / "shaders", queue);
          Collect(selfDir / "payload", queue);
          if (queue.empty()) {
            phase = Phase::Fail;
            lastError = "No game files next to Setup";
          } else {
            std::error_code ec;
            fs::create_directories(pathEdit, ec);
            phase = Phase::Installing;
            status = "Installing...";
          }
        }
      }
      if (Btn(Rectangle{270, 400, 160, 48}, "Cancel", false)) break;
    } else if (phase == Phase::Installing) {
      if (qi < queue.size()) {
        fs::path src = queue[qi];
        std::string s = src.string(), root = selfDir.string(), rel;
        if (s.find(root) == 0) rel = s.substr(root.size());
        else rel = src.filename().string();
        while (!rel.empty() && (rel[0] == '/' || rel[0] == '\\')) rel.erase(0, 1);
        if (rel.rfind("payload/", 0) == 0) rel = rel.substr(8);
        if (rel.rfind("payload\\", 0) == 0) rel = rel.substr(8);
        fs::path dst = fs::path(pathEdit) / rel;
        std::error_code ec;
        fs::create_directories(dst.parent_path(), ec);
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
        if (ec) {
          phase = Phase::Fail;
          lastError = "Copy failed: " + src.string();
        } else {
          qi++;
          progress = (float)qi / (float)queue.size();
          status = "Copy " + std::to_string(qi) + " / " + std::to_string(queue.size());
        }
      } else {
        std::ofstream(fs::path(pathEdit) / "installed.txt") << "ok\n";
        phase = Phase::Done;
        if (makeShortcut) {
          fs::path exe = fs::path(pathEdit) / "botbandit.exe";
          if (fs::exists(exe))
            CreateDesktopShortcut(exe.string(), pathEdit, "BOT BANDIT UNDERWORLD");
        }
      }
      DrawText("Installing...", 48, 150, 22, CTxt());
      DrawText(status.c_str(), 48, 190, 16, CMut());
      Rectangle bar{48, 240, (float)W - 120.f, 28};
      DrawRectangleRounded(bar, 0.2f, 6, Color{16, 16, 24, 255});
      DrawRectangleRounded(Rectangle{bar.x, bar.y, bar.width * progress, bar.height}, 0.2f, 6, CAcc());
      DrawRectangleRoundedLines(bar, 0.2f, 6, Color{80, 80, 100, 255});
      char pct[16];
      snprintf(pct, sizeof(pct), "%d%%", (int)(progress * 100));
      DrawText(pct, (int)(bar.x + bar.width * 0.5f - 16), (int)(bar.y + 5), 18, CTxt());
    } else if (phase == Phase::Done) {
      DrawText("Installation complete", 48, 160, 26, COk());
      DrawText(pathEdit.c_str(), 48, 200, 16, CMut());
      if (Btn(Rectangle{48, 400, 200, 48}, "Finish", true)) {
        if (launchAfter) {
          fs::path exe = fs::path(pathEdit) / "botbandit.exe";
          if (fs::exists(exe)) LaunchInstalledGame(exe.string(), pathEdit);
        }
        break;
      }
    } else {
      DrawText("Installation failed", 48, 160, 26, CAcc());
      DrawText(lastError.c_str(), 48, 200, 16, CMut());
      if (Btn(Rectangle{48, 400, 200, 48}, "Close", true)) break;
    }

    DrawCircleV(mouse, 5.f, WHITE);
    DrawCircleV(mouse, 3.5f, CAcc());
    DrawCircleLines((int)mouse.x, (int)mouse.y, 12.f, Color{255, 255, 255, 180});
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
