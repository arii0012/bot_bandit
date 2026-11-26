#pragma once
#include <string>
#include <atomic>

// Online installer helpers — no elevation (asInvoker / LOCALAPPDATA).
namespace WinAdmin {
  // Override via BOTBANDIT_RELEASE_URL env or edit this default for your repo.
  inline const char* DefaultReleaseZipUrl() {
    return "https://github.com/OWNER/REPO/releases/latest/download/BotBandit-Windows-x64.zip";
  }

  std::string DefaultInstallDir();
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
