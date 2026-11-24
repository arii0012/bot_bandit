#pragma once
#include <string>

bool IsRunningAsAdmin();
bool RelaunchAsAdmin();
bool NeedsAdminPath(const std::string& path);
std::string BrowseFolderDialog();
bool CreateDesktopShortcut(const std::string& targetExe, const std::string& workDir, const std::string& linkName);
void LaunchInstalledGame(const std::string& exePath, const std::string& workDir);
std::string GetModuleDirectory();
