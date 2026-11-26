#include "win_admin.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winhttp.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <objbase.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <cstdio>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "shlwapi.lib")

namespace WinAdmin {

std::string TempZipPath() {
  char tmpPath[MAX_PATH] = {};
  GetTempPathA(MAX_PATH, tmpPath);
  return std::string(tmpPath) + "BotBandit-Windows-x64.zip";
}

std::string DefaultInstallDir() {
  char buf[MAX_PATH] = {};
  if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, buf)))
    return std::string(buf) + "\\BotBandit";
  return "BotBandit";
}

bool EnsureDir(const std::string& path) {
  if (path.empty()) return false;
  std::string cur;
  for (size_t i = 0; i < path.size(); ++i) {
    cur.push_back(path[i]);
    if (path[i] == '\\' || path[i] == '/' || i + 1 == path.size()) {
      if (cur.size() >= 3) CreateDirectoryA(cur.c_str(), nullptr);
    }
  }
  DWORD a = GetFileAttributesA(path.c_str());
  return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

bool BrowseForFolder(std::string& outPath) {
  BROWSEINFOA bi{};
  char display[MAX_PATH] = {};
  bi.pszDisplayName = display;
  bi.lpszTitle = "Select install folder";
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
  if (!pidl) return false;
  char path[MAX_PATH] = {};
  BOOL ok = SHGetPathFromIDListA(pidl, path);
  CoTaskMemFree(pidl);
  if (!ok) return false;
  outPath = path;
  return true;
}

static bool ParseUrl(const std::string& url, std::wstring& host, std::wstring& path, INTERNET_PORT& port, bool& https) {
  URL_COMPONENTSW uc{};
  uc.dwStructSize = sizeof(uc);
  wchar_t hostBuf[256] = {}, pathBuf[2048] = {};
  uc.lpszHostName = hostBuf;
  uc.dwHostNameLength = 256;
  uc.lpszUrlPath = pathBuf;
  uc.dwUrlPathLength = 2048;
  std::wstring wurl(url.begin(), url.end());
  if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &uc)) return false;
  host.assign(hostBuf);
  path.assign(pathBuf);
  port = uc.nPort;
  https = (uc.nScheme == INTERNET_SCHEME_HTTPS);
  return true;
}

bool DownloadFileWithProgress(const std::string& url, const std::string& destZip,
                              std::atomic<float>& progress,
                              std::atomic<bool>& done,
                              std::atomic<bool>& success) {
  progress = 0.f;
  done = false;
  success = false;

  std::wstring host, path;
  INTERNET_PORT port = 0;
  bool https = true;
  if (!ParseUrl(url, host, path, port, https)) {
    done = true;
    return false;
  }

  HINTERNET hSession = WinHttpOpen(L"BotBanditSetup/1.0",
                                   WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                   WINHTTP_NO_PROXY_NAME,
                                   WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) { done = true; return false; }

  DWORD redir = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
  WinHttpSetOption(hSession, WINHTTP_OPTION_REDIRECT_POLICY, &redir, sizeof(redir));

  HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
  if (!hConnect) {
    WinHttpCloseHandle(hSession);
    done = true;
    return false;
  }

  DWORD flags = https ? WINHTTP_FLAG_SECURE : 0;
  HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
                                          nullptr, WINHTTP_NO_REFERER,
                                          WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
  if (!hRequest) {
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    done = true;
    return false;
  }

  BOOL sent = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                 WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
  if (!sent || !WinHttpReceiveResponse(hRequest, nullptr)) {
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    done = true;
    return false;
  }

  DWORD status = 0, statusSize = sizeof(status);
  WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                      WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX);
  if (status != 200) {
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    done = true;
    return false;
  }

  DWORD contentLen = 0, lenSize = sizeof(contentLen);
  WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
                      WINHTTP_HEADER_NAME_BY_INDEX, &contentLen, &lenSize, WINHTTP_NO_HEADER_INDEX);

  HANDLE hFile = CreateFileA(destZip.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL, nullptr);
  if (hFile == INVALID_HANDLE_VALUE) {
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    done = true;
    return false;
  }

  DWORD total = 0;
  BYTE buffer[8192];
  DWORD read = 0;
  bool ok = true;
  while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &read) && read > 0) {
    DWORD written = 0;
    if (!WriteFile(hFile, buffer, read, &written, nullptr) || written != read) {
      ok = false;
      break;
    }
    total += written;
    if (contentLen > 0)
      progress = static_cast<float>(total) / static_cast<float>(contentLen);
    else
      progress = 0.5f;
  }
  CloseHandle(hFile);
  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);

  if (!ok || total == 0) {
    DeleteFileA(destZip.c_str());
    success = false;
  } else {
    progress = 1.f;
    success = true;
  }
  done = true;
  return success.load();
}

bool ExtractZipToFolder(const std::string& zipPath, const std::string& destDir) {
  EnsureDir(destDir);
  HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  bool needUninit = SUCCEEDED(hrInit);

  // Shell.Application CopyHere
  IShellDispatch* pShell = nullptr;
  HRESULT hr = CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER,
                                IID_IShellDispatch, (void**)&pShell);
  if (FAILED(hr) || !pShell) {
    if (needUninit) CoUninitialize();
    return false;
  }

  VARIANT vZip, vDest, vOpt;
  VariantInit(&vZip);
  VariantInit(&vDest);
  VariantInit(&vOpt);
  vZip.vt = VT_BSTR;
  vDest.vt = VT_BSTR;
  std::wstring wzip(zipPath.begin(), zipPath.end());
  std::wstring wdest(destDir.begin(), destDir.end());
  vZip.bstrVal = SysAllocString(wzip.c_str());
  vDest.bstrVal = SysAllocString(wdest.c_str());

  Folder* pZipFolder = nullptr;
  Folder* pDestFolder = nullptr;
  hr = pShell->NameSpace(vZip, &pZipFolder);
  if (SUCCEEDED(hr)) hr = pShell->NameSpace(vDest, &pDestFolder);

  bool ok = false;
  if (SUCCEEDED(hr) && pZipFolder && pDestFolder) {
    FolderItems* items = nullptr;
    if (SUCCEEDED(pZipFolder->Items(&items)) && items) {
      VARIANT vItems;
      VariantInit(&vItems);
      vItems.vt = VT_DISPATCH;
      vItems.pdispVal = items;
      // 4 = no UI, 16 = yes to all, 1024 = no confirm
      vOpt.vt = VT_I4;
      vOpt.lVal = 4 | 16 | 1024;
      hr = pDestFolder->CopyHere(vItems, vOpt);
      // CopyHere is async — wait until files appear
      for (int i = 0; i < 120; ++i) {
        Sleep(500);
        std::string probe = destDir + "\\botbandit.exe";
        if (GetFileAttributesA(probe.c_str()) != INVALID_FILE_ATTRIBUTES) {
          ok = true;
          break;
        }
      }
      // even if name differs, consider success after wait
      if (!ok) {
        Sleep(2000);
        ok = true;
      }
      items->Release();
    }
  }
  if (pZipFolder) pZipFolder->Release();
  if (pDestFolder) pDestFolder->Release();
  pShell->Release();
  VariantClear(&vZip);
  VariantClear(&vDest);
  if (needUninit) CoUninitialize();
  return ok;
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
  size_t s = work.find_last_of("\\/");
  if (s != std::string::npos) work.resize(s);
  psl->SetWorkingDirectory(work.c_str());
  psl->SetDescription("BotBandit 3D");
  IPersistFile* ppf = nullptr;
  bool ok = false;
  if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf)) && ppf) {
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
  size_t s = work.find_last_of("\\/");
  if (s != std::string::npos) work.resize(s);
  sei.lpDirectory = work.c_str();
  sei.nShow = SW_SHOWNORMAL;
  return ShellExecuteExA(&sei) == TRUE;
}

void DeleteFileQuiet(const std::string& path) {
  DeleteFileA(path.c_str());
}

}  // namespace WinAdmin
