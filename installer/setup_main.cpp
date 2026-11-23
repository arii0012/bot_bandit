#include "raylib.h"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <algorithm>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#include <shobjidl.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "uuid.lib")
#endif

namespace fs = std::filesystem;

static Color ColBG() { return {10, 10, 16, 255}; }
static Color ColPanel() { return {22, 22, 32, 230}; }
static Color ColAccent() { return {220, 40, 70, 255}; }
static Color ColBtn() { return {40, 40, 55, 255}; }
static Color ColBtnH() { return {60, 50, 75, 255}; }
static Color ColText() { return {240, 240, 245, 255}; }
static Color ColMuted() { return {140, 140, 155, 255}; }
static Color ColOk() { return {40, 180, 100, 255}; }

static bool Btn(Rectangle r, const char* label, bool primary) {
  Vector2 m = GetMousePosition();
  bool h = CheckCollisionPointRec(m, r);
  Color c = primary ? (h ? Color{255, 60, 90, 255} : ColAccent()) : (h ? ColBtnH() : ColBtn());
  DrawRectangleRounded(r, 0.25f, 8, c);
  DrawRectangleRoundedLines(r, 0.25f, 8, h ? WHITE : Color{80, 80, 100, 255});
  int tw = MeasureText(label, 20);
  DrawText(label, (int)(r.x + (r.width - tw) * 0.5f), (int)(r.y + (r.height - 20) * 0.5f), 20, ColText());
  return h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

static void Checkbox(Rectangle r, const char* label, bool* on) {
  Vector2 m = GetMousePosition();
  bool h = CheckCollisionPointRec(m, r);
  DrawRectangleRounded({r.x, r.y, 22, 22}, 0.2f, 4, h ? ColBtnH() : ColBtn());
  DrawRectangleRoundedLines({r.x, r.y, 22, 22}, 0.2f, 4, *on ? ColOk() : Color{80, 80, 100, 255});
  if (*on) {
    DrawLineEx({r.x + 5, r.y + 11}, {r.x + 9, r.y + 16}, 2.5f, ColOk());
    DrawLineEx({r.x + 9, r.y + 16}, {r.x + 17, r.y + 6}, 2.5f, ColOk());
  }
  DrawText(label, (int)(r.x + 30), (int)(r.y + 3), 18, ColText());
  if (h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *on = !*on;
}

static bool PathLooksSafe(const std::string& p) {
  if (p.size() < 3) return false;
#if defined(_WIN32)
  if (p[1] != ':') return false;
#endif
  return true;
}

static std::string DefaultInstallDir() {
#if defined(_WIN32)
  return "C:\\Games\\LifeSim3D";
#else
  return std::string(getenv("HOME") ? getenv("HOME") : ".") + "/Games/LifeSim3D";
#endif
}

#if defined(_WIN32)
static std::string BrowseFolder(HWND owner) {
  char path[MAX_PATH] = {};
  BROWSEINFOA bi{};
  bi.hwndOwner = owner;
  bi.lpszTitle = "Select install folder";
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  PIDLIST_ABSOLUTE pidl = SHBrowseForFolderA(&bi);
  if (!pidl) return {};
  if (SHGetPathFromIDListA(pidl, path)) {
    CoTaskMemFree(pidl);
    return std::string(path);
  }
  CoTaskMemFree(pidl);
  return {};
}

static bool CreateDesktopShortcut(const std::string& exePath, const std::string& name) {
  HRESULT hr = CoInitialize(nullptr);
  bool needUninit = SUCCEEDED(hr);
  IShellLinkA* psl = nullptr;
  hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void**)&psl);
  if (FAILED(hr)) {
    if (needUninit) CoUninitialize();
    return false;
  }
  psl->SetPath(exePath.c_str());
  fs::path dir = fs::path(exePath).parent_path();
  psl->SetWorkingDirectory(dir.string().c_str());
  psl->SetDescription("BOT BANDIT: UNDERWORLD");
  IPersistFile* ppf = nullptr;
  hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
  bool ok = false;
  if (SUCCEEDED(hr)) {
    char desktop[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, desktop))) {
      std::string lnk = std::string(desktop) + "\\" + name + ".lnk";
      wchar_t wpath[MAX_PATH];
      MultiByteToWideChar(CP_UTF8, 0, lnk.c_str(), -1, wpath, MAX_PATH);
      ok = SUCCEEDED(ppf->Save(wpath, TRUE));
    }
    ppf->Release();
  }
  psl->Release();
  if (needUninit) CoUninitialize();
  return ok;
}
#endif

static void CollectFiles(const fs::path& root, std::vector<fs::path>& out) {
  if (!fs::exists(root)) return;
  for (auto& e : fs::recursive_directory_iterator(root)) {
    if (e.is_regular_file()) out.push_back(e.path());
  }
}

static bool CopyOne(const fs::path& from, const fs::path& to) {
  std::error_code ec;
  fs::create_directories(to.parent_path(), ec);
  fs::copy_file(from, to, fs::copy_options::overwrite_existing, ec);
  return !ec;
}

enum class Phase { Form, Installing, Done, Fail };

int main() {
  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_ALWAYS_RUN);
  const int W = 780, H = 520;
  InitWindow(W, H, "LifeSim3D Setup");
  SetTargetFPS(60);
  HideCursor();

  std::string installPath = DefaultInstallDir();
  bool makeShortcut = true;
  bool launchAfter = true;
  Phase phase = Phase::Form;
  std::string status = "Ready";
  float progress = 0.f;
  std::vector<fs::path> queue;
  size_t qi = 0;
  fs::path payloadRoot;
  std::string lastError;
  float anim = 0.f;
  std::string pathEdit = installPath;
  bool editing = false;

  // Payload: next to setup exe — botbandit.exe + assets + shaders
  fs::path selfDir = fs::current_path();
#if defined(_WIN32)
  {
    char mod[MAX_PATH];
    if (GetModuleFileNameA(nullptr, mod, MAX_PATH))
      selfDir = fs::path(mod).parent_path();
  }
#endif
  payloadRoot = selfDir;

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    anim += dt;
    Vector2 mouse = GetMousePosition();

    BeginDrawing();
    ClearBackground(ColBG());

    // glass panel
    DrawRectangleRounded({24, 24, W - 48.f, H - 48.f}, 0.06f, 12, ColPanel());
    DrawRectangleRoundedLines({24, 24, W - 48.f, H - 48.f}, 0.06f, 12, ColAccent());

    DrawText("LifeSim3D / BOT BANDIT", 48, 44, 28, ColText());
    DrawText("UNDERWORLD  Setup", 48, 78, 18, ColMuted());

    if (phase == Phase::Form) {
      DrawText("Install folder:", 48, 130, 18, ColMuted());
      Rectangle pathBox = {48, 158, 520, 40};
      DrawRectangleRounded(pathBox, 0.15f, 6, Color{16, 16, 24, 255});
      DrawRectangleRoundedLines(pathBox, 0.15f, 6, editing ? ColAccent() : Color{70, 70, 90, 255});
      if (CheckCollisionPointRec(mouse, pathBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        editing = true;
      if (editing) {
        int ch = GetCharPressed();
        while (ch > 0) {
          if (ch >= 32 && ch < 127 && pathEdit.size() < 240) pathEdit.push_back((char)ch);
          ch = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !pathEdit.empty()) pathEdit.pop_back();
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) editing = false;
      }
      DrawText(pathEdit.c_str(), 58, 168, 18, ColText());

      if (Btn({580, 158, 140, 40}, "Browse...", false)) {
        editing = false;
#if defined(_WIN32)
        std::string picked = BrowseFolder(GetWindowHandle() ? (HWND)0 : nullptr);
        if (!picked.empty()) {
          pathEdit = picked + "\\LifeSim3D";
          installPath = pathEdit;
        }
#else
        status = "Browse available on Windows";
#endif
      }

      Checkbox({48, 220, 400, 28}, "Create desktop shortcut", &makeShortcut);
      Checkbox({48, 260, 400, 28}, "Launch game after install", &launchAfter);

      DrawText(status.c_str(), 48, 320, 16, ColMuted());

      if (Btn({48, 400, 200, 48}, "Install", true)) {
        installPath = pathEdit;
        if (!PathLooksSafe(installPath)) {
          status = "Invalid path";
        } else {
          queue.clear();
          qi = 0;
          // Prefer files next to installer
          std::vector<std::string> names = {
            "botbandit.exe", "botbandit",
          };
          for (auto& n : names) {
            fs::path p = payloadRoot / n;
            if (fs::exists(p)) queue.push_back(p);
          }
          CollectFiles(payloadRoot / "assets", queue);
          CollectFiles(payloadRoot / "shaders", queue);
          // also allow payload/ subfolder layout
          CollectFiles(payloadRoot / "payload", queue);
          if (queue.empty()) {
            phase = Phase::Fail;
            lastError = "No game files next to Setup (botbandit.exe / assets)";
          } else {
            phase = Phase::Installing;
            status = "Installing...";
            std::error_code ec;
            fs::create_directories(installPath, ec);
          }
        }
      }
      if (Btn({270, 400, 160, 48}, "Cancel", false)) break;
    }
    else if (phase == Phase::Installing) {
      if (qi < queue.size()) {
        fs::path src = queue[qi];
        fs::path rel;
        std::string s = src.string();
        std::string root = payloadRoot.string();
        if (s.find(root) == 0) rel = s.substr(root.size());
        else rel = src.filename();
        while (!rel.empty() && (rel.string()[0] == '/' || rel.string()[0] == '\\'))
          rel = rel.string().substr(1);
        // strip payload/ prefix
        std::string rs = rel.string();
        if (rs.rfind("payload/", 0) == 0) rs = rs.substr(8);
        if (rs.rfind("payload\\", 0) == 0) rs = rs.substr(8);
        fs::path dst = fs::path(installPath) / rs;
        if (!CopyOne(src, dst)) {
          phase = Phase::Fail;
          lastError = "Copy failed: " + src.string();
        } else {
          qi++;
          progress = queue.empty() ? 1.f : (float)qi / (float)queue.size();
          status = "Copy " + std::to_string(qi) + " / " + std::to_string(queue.size());
        }
      } else {
        // write marker
        std::ofstream mk(fs::path(installPath) / "installed.txt");
        mk << "LifeSim3D installed\n";
        phase = Phase::Done;
        status = "Done";
#if defined(_WIN32)
        if (makeShortcut) {
          fs::path exe = fs::path(installPath) / "botbandit.exe";
          if (fs::exists(exe))
            CreateDesktopShortcut(exe.string(), "BOT BANDIT UNDERWORLD");
        }
#endif
      }
      DrawText("Installing...", 48, 150, 22, ColText());
      DrawText(status.c_str(), 48, 190, 16, ColMuted());
      Rectangle bar = {48, 240, W - 120.f, 28};
      DrawRectangleRounded(bar, 0.2f, 6, Color{16, 16, 24, 255});
      DrawRectangleRounded({bar.x, bar.y, bar.width * progress, bar.height}, 0.2f, 6, ColAccent());
      DrawRectangleRoundedLines(bar, 0.2f, 6, Color{80, 80, 100, 255});
      char pct[16];
      snprintf(pct, sizeof(pct), "%d%%", (int)(progress * 100));
      DrawText(pct, (int)(bar.x + bar.width * 0.5f - 16), (int)(bar.y + 5), 18, ColText());
    }
    else if (phase == Phase::Done) {
      DrawText("Installation complete", 48, 160, 26, ColOk());
      DrawText(installPath.c_str(), 48, 200, 16, ColMuted());
      if (Btn({48, 400, 200, 48}, "Finish", true)) {
#if defined(_WIN32)
        if (launchAfter) {
          fs::path exe = fs::path(installPath) / "botbandit.exe";
          if (fs::exists(exe)) {
            std::string cmd = "\"" + exe.string() + "\"";
            STARTUPINFOA si{};
            PROCESS_INFORMATION pi{};
            si.cb = sizeof(si);
            std::vector<char> buf(cmd.begin(), cmd.end());
            buf.push_back(0);
            CreateProcessA(nullptr, buf.data(), nullptr, nullptr, FALSE, 0, nullptr,
                           installPath.c_str(), &si, &pi);
            if (pi.hProcess) {
              CloseHandle(pi.hProcess);
              CloseHandle(pi.hThread);
            }
          }
        }
#endif
        break;
      }
    }
    else if (phase == Phase::Fail) {
      DrawText("Installation failed", 48, 160, 26, ColAccent());
      DrawText(lastError.c_str(), 48, 200, 16, ColMuted());
      if (Btn({48, 400, 200, 48}, "Close", true)) break;
    }

    // custom cursor
    DrawCircleV(mouse, 5.f, WHITE);
    DrawCircleV(mouse, 3.5f, ColAccent());
    DrawCircleLines((int)mouse.x, (int)mouse.y, 12.f, Color{255, 255, 255, 180});

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
