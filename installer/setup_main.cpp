#include "raylib.h"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cctype>

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
#pragma comment(lib, "advapi32.lib")
#endif

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
  return h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

static void Checkbox(Rectangle r, const char* label, bool* on) {
  Vector2 m = GetMousePosition();
  bool h = CheckCollisionPointRec(m, r);
  DrawRectangleRounded({r.x, r.y, 22, 22}, 0.2f, 4, h ? CBtnH() : CBtn());
  DrawRectangleRoundedLines({r.x, r.y, 22, 22}, 0.2f, 4, *on ? COk() : Color{80, 80, 100, 255});
  if (*on) {
    DrawLineEx({r.x + 5, r.y + 11}, {r.x + 9, r.y + 16}, 2.5f, COk());
    DrawLineEx({r.x + 9, r.y + 16}, {r.x + 17, r.y + 6}, 2.5f, COk());
  }
  DrawText(label, (int)(r.x + 30), (int)(r.y + 3), 18, CTxt());
  if (h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *on = !*on;
}

static std::string DefaultDir() {
#if defined(_WIN32)
  return "C:\\Games\\LifeSim3D";
#else
  const char* h = getenv("HOME");
  return std::string(h ? h : ".") + "/Games/LifeSim3D";
#endif
}

#if defined(_WIN32)
static bool IsAdmin() {
  BOOL admin = FALSE;
  PSID group = nullptr;
  SID_IDENTIFIER_AUTHORITY nt = SECURITY_NT_AUTHORITY;
  if (AllocateAndInitializeSid(&nt, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                               0, 0, 0, 0, 0, 0, &group)) {
    CheckTokenMembership(nullptr, group, &admin);
    FreeSid(group);
  }
  return admin == TRUE;
}

static bool RelaunchElevated() {
  char path[MAX_PATH];
  if (!GetModuleFileNameA(nullptr, path, MAX_PATH)) return false;
  SHELLEXECUTEINFOA sei{};
  sei.cbSize = sizeof(sei);
  sei.lpVerb = "runas";
  sei.lpFile = path;
  sei.nShow = SW_SHOWNORMAL;
  return ShellExecuteExA(&sei) == TRUE;
}

static bool NeedsAdminPath(const std::string& p) {
  std::string low = p;
  for (char& c : low) c = (char)tolower((unsigned char)c);
  return low.find("program files") != std::string::npos || low.find(":\\windows") != std::string::npos;
}

static std::string BrowseFolder() {
  char path[MAX_PATH] = {};
  BROWSEINFOA bi{};
  bi.lpszTitle = "Select install folder";
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  PIDLIST_ABSOLUTE pidl = SHBrowseForFolderA(&bi);
  if (!pidl) return {};
  if (!SHGetPathFromIDListA(pidl, path)) { CoTaskMemFree(pidl); return {}; }
  CoTaskMemFree(pidl);
  return std::string(path);
}

static bool CreateDesktopShortcut(const std::string& exePath, const std::string& name) {
  HRESULT hr = CoInitialize(nullptr);
  bool un = SUCCEEDED(hr);
  IShellLinkA* psl = nullptr;
  hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void**)&psl);
  if (FAILED(hr)) { if (un) CoUninitialize(); return false; }
  psl->SetPath(exePath.c_str());
  psl->SetWorkingDirectory(fs::path(exePath).parent_path().string().c_str());
  psl->SetDescription("BOT BANDIT: UNDERWORLD");
  IPersistFile* ppf = nullptr;
  bool ok = false;
  if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf))) {
    char desktop[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, desktop))) {
      std::string lnk = std::string(desktop) + "\\" + name + ".lnk";
      wchar_t w[MAX_PATH];
      MultiByteToWideChar(CP_UTF8, 0, lnk.c_str(), -1, w, MAX_PATH);
      ok = SUCCEEDED(ppf->Save(w, TRUE));
    }
    ppf->Release();
  }
  psl->Release();
  if (un) CoUninitialize();
  return ok;
}
#endif

static void Collect(const fs::path& root, std::vector<fs::path>& out) {
  if (!fs::exists(root)) return;
  for (auto& e : fs::recursive_directory_iterator(root))
    if (e.is_regular_file()) out.push_back(e.path());
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
  std::string status = "Ready", lastError;
  float progress = 0.f;
  std::vector<fs::path> queue;
  size_t qi = 0;
  bool editing = false;

  fs::path selfDir = fs::current_path();
#if defined(_WIN32)
  char mod[MAX_PATH];
  if (GetModuleFileNameA(nullptr, mod, MAX_PATH)) selfDir = fs::path(mod).parent_path();
  status = IsAdmin() ? "Running as Administrator" : "User mode (OK for C:\\Games)";
#endif

  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    BeginDrawing();
    ClearBackground(CBg());
    DrawRectangleRounded({24, 24, W - 48.f, H - 48.f}, 0.06f, 12, CPanel());
    DrawRectangleRoundedLines({24, 24, W - 48.f, H - 48.f}, 0.06f, 12, CAcc());
    DrawText("LifeSim3D / BOT BANDIT", 48, 44, 28, CTxt());
    DrawText("UNDERWORLD  Setup", 48, 78, 18, CMut());

    if (phase == Phase::Form) {
      DrawText("Install folder:", 48, 130, 18, CMut());
      Rectangle pathBox = {48, 158, 520, 40};
      DrawRectangleRounded(pathBox, 0.15f, 6, Color{16, 16, 24, 255});
      DrawRectangleRoundedLines(pathBox, 0.15f, 6, editing ? CAcc() : Color{70, 70, 90, 255});
      if (CheckCollisionPointRec(mouse, pathBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) editing = true;
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
      if (Btn({580, 158, 140, 40}, "Browse...", false)) {
        editing = false;
#if defined(_WIN32)
        std::string p = BrowseFolder();
        if (!p.empty()) pathEdit = p + "\\LifeSim3D";
#endif
      }
      Checkbox({48, 220, 400, 28}, "Create desktop shortcut", &makeShortcut);
      Checkbox({48, 260, 400, 28}, "Launch game after install", &launchAfter);
      DrawText(status.c_str(), 48, 320, 16, CMut());
      if (Btn({48, 400, 200, 48}, "Install", true)) {
#if defined(_WIN32)
        if (NeedsAdminPath(pathEdit) && !IsAdmin()) {
          status = "Requesting Administrator (UAC)...";
          if (RelaunchElevated()) break;
          status = "UAC denied — choose C:\\Games\\LifeSim3D";
        } else
#endif
        {
          queue.clear(); qi = 0;
          for (const char* n : {"botbandit.exe", "botbandit"}) {
            fs::path p = selfDir / n;
            if (fs::exists(p)) queue.push_back(p);
          }
          Collect(selfDir / "assets", queue);
          Collect(selfDir / "shaders", queue);
          Collect(selfDir / "payload", queue);
          if (queue.empty()) { phase = Phase::Fail; lastError = "No game files next to Setup"; }
          else {
            std::error_code ec;
            fs::create_directories(pathEdit, ec);
            phase = Phase::Installing;
            status = "Installing...";
          }
        }
      }
      if (Btn({270, 400, 160, 48}, "Cancel", false)) break;
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
        if (ec) { phase = Phase::Fail; lastError = "Copy failed: " + src.string(); }
        else {
          qi++;
          progress = (float)qi / (float)queue.size();
          status = "Copy " + std::to_string(qi) + " / " + std::to_string(queue.size());
        }
      } else {
        std::ofstream(fs::path(pathEdit) / "installed.txt") << "ok\n";
        phase = Phase::Done;
#if defined(_WIN32)
        if (makeShortcut) {
          fs::path exe = fs::path(pathEdit) / "botbandit.exe";
          if (fs::exists(exe)) CreateDesktopShortcut(exe.string(), "BOT BANDIT UNDERWORLD");
        }
#endif
      }
      DrawText("Installing...", 48, 150, 22, CTxt());
      DrawText(status.c_str(), 48, 190, 16, CMut());
      Rectangle bar = {48, 240, W - 120.f, 28};
      DrawRectangleRounded(bar, 0.2f, 6, Color{16, 16, 24, 255});
      DrawRectangleRounded({bar.x, bar.y, bar.width * progress, bar.height}, 0.2f, 6, CAcc());
      DrawRectangleRoundedLines(bar, 0.2f, 6, Color{80, 80, 100, 255});
      char pct[16];
      snprintf(pct, sizeof(pct), "%d%%", (int)(progress * 100));
      DrawText(pct, (int)(bar.x + bar.width * 0.5f - 16), (int)(bar.y + 5), 18, CTxt());
    } else if (phase == Phase::Done) {
      DrawText("Installation complete", 48, 160, 26, COk());
      DrawText(pathEdit.c_str(), 48, 200, 16, CMut());
      if (Btn({48, 400, 200, 48}, "Finish", true)) {
#if defined(_WIN32)
        if (launchAfter) {
          fs::path exe = fs::path(pathEdit) / "botbandit.exe";
          if (fs::exists(exe)) {
            std::string cmd = "\"" + exe.string() + "\"";
            STARTUPINFOA si{}; PROCESS_INFORMATION pi{}; si.cb = sizeof(si);
            std::vector<char> buf(cmd.begin(), cmd.end()); buf.push_back(0);
            CreateProcessA(nullptr, buf.data(), nullptr, nullptr, FALSE, 0, nullptr, pathEdit.c_str(), &si, &pi);
            if (pi.hProcess) { CloseHandle(pi.hProcess); CloseHandle(pi.hThread); }
          }
        }
#endif
        break;
      }
    } else {
      DrawText("Installation failed", 48, 160, 26, CAcc());
      DrawText(lastError.c_str(), 48, 200, 16, CMut());
      if (Btn({48, 400, 200, 48}, "Close", true)) break;
    }

    DrawCircleV(mouse, 5.f, WHITE);
    DrawCircleV(mouse, 3.5f, CAcc());
    DrawCircleLines((int)mouse.x, (int)mouse.y, 12.f, Color{255, 255, 255, 180});
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
