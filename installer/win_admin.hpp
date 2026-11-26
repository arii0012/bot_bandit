#pragma once
#include <string>
#include <atomic>

// Online installer helpers — no elevation (asInvoker / LOCALAPPDATA).
// No WinAPI headers here — they conflict with raylib (CloseWindow, ShowCursor, Rectangle, DrawText).
namespace WinAdmin {
  inline const char* DefaultReleaseZipUrl() {
    return "https://github.com/arii0012/bot_bandit/releases/latest/download/BotBandit-Windows-x64.zip";
  }

  std::string DefaultInstallDir();
  std::string TempZipPath();
  bool EnsureDir(const std::string& path);
  bool BrowseForFolder(std::string& outPath);
  bool DownloadFileWithProgress(const std::string& url, const std::string& destZip,
                                std::atomic<float>& progress,
                                std::atomic<bool>& done,
                                std::atomic<bool>& success);
  bool ExtractZipToFolder(const std::string& zipPath, const std::string& destDir);
  bool CreateDesktopShortcut(const std::string& exePath, const std::string& name);
  bool LaunchExe(const std::string& exePath);
  void DeleteFileQuiet(const std::string& path);
}
