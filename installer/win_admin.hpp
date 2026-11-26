#pragma once
#include <string>

// No elevation. Install under %LOCALAPPDATA%\BotBandit only.
namespace WinAdmin {
  std::string DefaultInstallDir();
  bool EnsureDir(const std::string& path);
  bool CopyTree(const std::string& src, const std::string& dst);
  bool CreateDesktopShortcut(const std::string& exePath, const std::string& name);
  bool LaunchExe(const std::string& exePath);
}
