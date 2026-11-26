#include "win_admin.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <objbase.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <cstdio>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

namespace WinAdmin {

std::string DefaultInstallDir() {
  char buf[MAX_PATH] = {};
  if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, buf))) {
    return std::string(buf) + "\\BotBandit";
  }
  return "BotBandit";
}

bool EnsureDir(const std::string& path) {
  if (path.empty()) return false;
  std::string cur;
  for (size_t i = 0; i < path.size(); ++i) {
    char c = path[i];
    cur.push_back(c);
    if (c == '\\' || c == '/' || i + 1 == path.size()) {
      if (cur.size() >= 3) CreateDirectoryA(cur.c_str(), nullptr);
    }
  }
  DWORD attr = GetFileAttributesA(path.c_str());
  return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

static bool CopyOne(const std::string& src, const std::string& dst) {
  return CopyFileA(src.c_str(), dst.c_str(), FALSE) != 0;
}

bool CopyTree(const std::string& src, const std::string& dst) {
  EnsureDir(dst);
  std::string pattern = src + "\\*";
  WIN32_FIND_DATAA fd{};
  HANDLE h = FindFirstFileA(pattern.c_str(), &fd);
  if (h == INVALID_HANDLE_VALUE) return false;
  do {
    if (fd.cFileName[0] == '.' && (fd.cFileName[1] == 0 || (fd.cFileName[1] == '.' && fd.cFileName[2] == 0)))
      continue;
    std::string from = src + "\\" + fd.cFileName;
    std::string to = dst + "\\" + fd.cFileName;
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      EnsureDir(to);
      CopyTree(from, to);
    } else {
      CopyOne(from, to);
    }
  } while (FindNextFileA(h, &fd));
  FindClose(h);
  return true;
}

bool CreateDesktopShortcut(const std::string& exePath, const std::string& name) {
  char desk[MAX_PATH] = {};
  if (FAILED(SHGetFolderPathA(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, desk)))
    return false;

  HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  bool needUninit = SUCCEEDED(hr);

  IShellLinkA* psl = nullptr;
  hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void**)&psl);
  if (FAILED(hr) || !psl) {
    if (needUninit) CoUninitialize();
    return false;
  }

  psl->SetPath(exePath.c_str());
  std::string work = exePath;
  size_t slash = work.find_last_of("\\/");
  if (slash != std::string::npos) work.resize(slash);
  psl->SetWorkingDirectory(work.c_str());
  psl->SetDescription("BotBandit 3D");

  IPersistFile* ppf = nullptr;
  hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
  bool ok = false;
  if (SUCCEEDED(hr) && ppf) {
    std::string link = std::string(desk) + "\\" + name + ".lnk";
    wchar_t wpath[MAX_PATH] = {};
    MultiByteToWideChar(CP_ACP, 0, link.c_str(), -1, wpath, MAX_PATH);
    ok = SUCCEEDED(ppf->Save(wpath, TRUE));
    ppf->Release();
  }
  psl->Release();
  if (needUninit) CoUninitialize();
  return ok;
}

bool LaunchExe(const std::string& exePath) {
  SHELLEXECUTEINFOA sei{};
  sei.cbSize = sizeof(sei);
  sei.fMask = SEE_MASK_NOCLOSEPROCESS;
  sei.lpVerb = "open";
  sei.lpFile = exePath.c_str();
  std::string work = exePath;
  size_t slash = work.find_last_of("\\/");
  if (slash != std::string::npos) work.resize(slash);
  sei.lpDirectory = work.c_str();
  sei.nShow = SW_SHOWNORMAL;
  return ShellExecuteExA(&sei) == TRUE;
}

}  // namespace WinAdmin
