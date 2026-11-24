#include "win_admin.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <objbase.h>
#include <shobjidl.h>
#include <cstring>
#include <vector>

bool IsRunningAsAdmin() {
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

bool RelaunchAsAdmin() {
  char path[MAX_PATH];
  if (!GetModuleFileNameA(nullptr, path, MAX_PATH)) return false;
  SHELLEXECUTEINFOA sei{};
  sei.cbSize = sizeof(sei);
  sei.lpVerb = "runas";
  sei.lpFile = path;
  sei.nShow = SW_SHOWNORMAL;
  return ShellExecuteExA(&sei) == TRUE;
}

bool NeedsAdminPath(const std::string& path) {
  std::string low = path;
  for (char& c : low) {
    if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
  }
  return low.find("program files") != std::string::npos || low.find(":\\windows") != std::string::npos;
}

std::string BrowseFolderDialog() {
  char path[MAX_PATH] = {};
  BROWSEINFOA bi{};
  bi.lpszTitle = "Select install folder";
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  PIDLIST_ABSOLUTE pidl = SHBrowseForFolderA(&bi);
  if (!pidl) return {};
  if (!SHGetPathFromIDListA(pidl, path)) {
    CoTaskMemFree(pidl);
    return {};
  }
  CoTaskMemFree(pidl);
  return std::string(path);
}

bool CreateDesktopShortcut(const std::string& targetExe, const std::string& workDir, const std::string& linkName) {
  HRESULT hr = CoInitialize(nullptr);
  bool un = SUCCEEDED(hr);
  IShellLinkA* psl = nullptr;
  hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void**)&psl);
  if (FAILED(hr)) {
    if (un) CoUninitialize();
    return false;
  }
  psl->SetPath(targetExe.c_str());
  psl->SetWorkingDirectory(workDir.c_str());
  psl->SetDescription("BOT BANDIT: UNDERWORLD");
  IPersistFile* ppf = nullptr;
  bool ok = false;
  if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf))) {
    char desktop[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, desktop))) {
      std::string lnk = std::string(desktop) + "\\" + linkName + ".lnk";
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

void LaunchInstalledGame(const std::string& exePath, const std::string& workDir) {
  std::string cmd = "\"" + exePath + "\"";
  STARTUPINFOA si{};
  PROCESS_INFORMATION pi{};
  si.cb = sizeof(si);
  std::vector<char> buf(cmd.begin(), cmd.end());
  buf.push_back(0);
  CreateProcessA(nullptr, buf.data(), nullptr, nullptr, FALSE, 0, nullptr, workDir.c_str(), &si, &pi);
  if (pi.hProcess) {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
}

std::string GetModuleDirectory() {
  char mod[MAX_PATH];
  if (!GetModuleFileNameA(nullptr, mod, MAX_PATH)) return ".";
  std::string p(mod);
  size_t pos = p.find_last_of("\\/");
  if (pos == std::string::npos) return ".";
  return p.substr(0, pos);
}

#else

bool IsRunningAsAdmin() { return true; }
bool RelaunchAsAdmin() { return false; }
bool NeedsAdminPath(const std::string&) { return false; }
std::string BrowseFolderDialog() { return {}; }
bool CreateDesktopShortcut(const std::string&, const std::string&, const std::string&) { return false; }
void LaunchInstalledGame(const std::string&, const std::string&) {}
std::string GetModuleDirectory() { return "."; }

#endif
