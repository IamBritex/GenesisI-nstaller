#pragma once
#include <windows.h>
#include <thread>
#include <dwmapi.h>
#include <psapi.h>
#include <filesystem>
#include <fstream>
#include <shlobj.h>
#include <string>
#include <system_error>
#include "Utils.h"
#include "Config.h"
#include "WebView2.h"
#include "Discord.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

namespace fs = std::filesystem;
extern AppConfig globalConfig;

class NativeBridge
{
private:
    static bool isTransparent;

public:
    static void HandleApiCall(HWND hWnd, std::wstring msgJSON, ICoreWebView2 *sender)
    {
        Utils::MiniJSON json(Utils::ToString(msgJSON));
        std::string group = json.get("group");
        std::string action = json.get("action");

        if (group == "window")
            HandleWindow(hWnd, action, json, sender);
        else if (group == "system")
            HandleSystem(hWnd, action, json, sender);
        else if (group == "fs")
            HandleFS(hWnd, action, json, sender);
        else if (group == "utils")
            HandleUtils(hWnd, action, json, sender);
        else if (group == "discord")
            HandleDiscord(hWnd, action, json, sender);
        else if (group == "installer")
            HandleInstaller(hWnd, action, json, sender);
    }

private:
    /**
     * @description Maneja la instalacion nativa del SDK
     */
    static void HandleInstaller(HWND hWnd, std::string action, Utils::MiniJSON &json, ICoreWebView2 *sender)
    {
        if (action == "run")
        {
            std::thread([=]()
                        {
                std::error_code ec;
                try {
                    fs::path installPath = L"C:\\Genesis";
                    wchar_t buffer[MAX_PATH];
                    if (GetModuleFileNameW(NULL, buffer, MAX_PATH) == 0) {
                        sender->PostWebMessageAsString(L"installer|ERROR_PATH");
                        return;
                    }
                    
                    fs::path sourceDir = fs::path(buffer).parent_path();
                    fs::create_directories(installPath, ec);
                    
                    if (ec) { 
                        sender->PostWebMessageAsString(L"installer|ERROR_DIR_CREATE"); 
                        return; 
                    }

                    auto opts = fs::copy_options::recursive | fs::copy_options::overwrite_existing;

                    // Proceso de copia con feedback de error especifico
                    if (fs::exists(sourceDir / "core")) {
                        fs::copy(sourceDir / "core", installPath / "core", opts, ec);
                        if (ec) { sender->PostWebMessageAsString(L"installer|ERROR_COPY_CORE"); return; }
                    } else {
                        sender->PostWebMessageAsString(L"installer|MISSING_CORE"); return;
                    }

                    if (fs::exists(sourceDir / "bin")) {
                        fs::copy(sourceDir / "bin", installPath / "bin", opts, ec);
                        if (ec) { sender->PostWebMessageAsString(L"installer|ERROR_COPY_BIN"); return; }
                    } else {
                        sender->PostWebMessageAsString(L"installer|MISSING_BIN"); return;
                    }

                    if (fs::exists(sourceDir / "builder.bat")) {
                        fs::copy(sourceDir / "builder.bat", installPath / "builder.bat", fs::copy_options::overwrite_existing, ec);
                    }

                    // Registro de Windows (PATH)
                    HKEY hKey;
                    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
                        wchar_t current[8192]; 
                        DWORD size = sizeof(current);
                        if (RegQueryValueExW(hKey, L"Path", NULL, NULL, (LPBYTE)current, &size) == ERROR_SUCCESS) {
                            std::wstring newP(current);
                            std::wstring bPath = L"C:\\Genesis\\bin";
                            if (newP.find(bPath) == std::wstring::npos) {
                                newP += L";" + bPath;
                                RegSetValueExW(hKey, L"Path", 0, REG_EXPAND_SZ, (LPBYTE)newP.c_str(), (DWORD)((newP.length() + 1) * sizeof(wchar_t)));
                                SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
                            }
                        }
                        RegCloseKey(hKey);
                    } else {
                        sender->PostWebMessageAsString(L"installer|ERROR_REGISTRY_ACCESS");
                        return;
                    }

                    sender->PostWebMessageAsString(L"installer|SUCCESS");
                } catch (...) {
                    sender->PostWebMessageAsString(L"installer|ERROR_FATAL");
                } })
                .detach();
        }
    }

    static void ApplyTransparency(HWND hWnd)
    {
        MARGINS margins = isTransparent ? MARGINS{-1} : MARGINS{0};
        DwmExtendFrameIntoClientArea(hWnd, &margins);
    }

    static void HandleWindow(HWND hWnd, std::string action, Utils::MiniJSON &json, ICoreWebView2 *sender)
    {
        RECT rc;
        GetWindowRect(hWnd, &rc);
        int w = rc.right - rc.left, h = rc.bottom - rc.top;
        int scrW = GetSystemMetrics(SM_CXSCREEN), scrH = GetSystemMetrics(SM_CYSCREEN);

        if (action == "close")
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        else if (action == "minimize")
            ShowWindow(hWnd, SW_MINIMIZE);
        else if (action == "maximize")
            ShowWindow(hWnd, SW_MAXIMIZE);
        else if (action == "restore")
            ShowWindow(hWnd, SW_RESTORE);
        else if (action == "drag")
        {
            ReleaseCapture();
            SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
        else if (action == "center")
            SetWindowPos(hWnd, NULL, (scrW - w) / 2, (scrH - h) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        else if (action == "setTransparent")
        {
            isTransparent = json.getBool("param");
            ApplyTransparency(hWnd);
            SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }
        else if (action == "setOpacity")
        {
            SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(hWnd, 0, (BYTE)json.getInt("param"), LWA_ALPHA);
        }
        else if (action == "setTitle")
        {
            std::wstring title = json.getW("param");
            SetWindowTextW(hWnd, title.c_str());
        }
    }

    static void HandleSystem(HWND hWnd, std::string action, Utils::MiniJSON &json, ICoreWebView2 *sender)
    {
        if (action == "isAdmin")
        {
            BOOL f = FALSE;
            HANDLE h = NULL;
            if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &h))
            {
                TOKEN_ELEVATION e;
                DWORD s;
                if (GetTokenInformation(h, TokenElevation, &e, sizeof(e), &s))
                    f = e.TokenIsElevated;
            }
            if (h)
                CloseHandle(h);
            sender->PostWebMessageAsString((L"isAdmin|" + std::wstring(f ? L"true" : L"false")).c_str());
        }
    }

    static void HandleFS(HWND hWnd, std::string action, Utils::MiniJSON &json, ICoreWebView2 *sender)
    {
        std::wstring root = Utils::GetAppDataPath(globalConfig.appID);
        std::wstring pathW = Utils::ToWString(json.get("path"));
        if (action == "list")
        {
            fs::path safePath;
            if (Utils::IsSafePath(root, pathW, safePath) && fs::is_directory(safePath))
            {
                std::wstring out = L"";
                for (auto &e : fs::directory_iterator(safePath))
                    out += e.path().filename().wstring() + (e.is_directory() ? L"/" : L"") + L";";
                sender->PostWebMessageAsString((L"fsList|" + pathW + L"|" + out).c_str());
            }
        }
    }

    static void HandleUtils(HWND hWnd, std::string action, Utils::MiniJSON &json, ICoreWebView2 *sender)
    {
        if (action == "notify")
        {
            MessageBoxW(hWnd, json.getW("msg").c_str(), json.getW("title").c_str(), MB_OK | MB_ICONINFORMATION);
        }
        else if (action == "openExternal")
        {
            ShellExecuteW(NULL, L"open", json.getW("param").c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }

    static void HandleDiscord(HWND hWnd, std::string action, Utils::MiniJSON &json, ICoreWebView2 *sender)
    {
        if (action == "setActivity")
        {
            DiscordManager::Get().SetActivity(
                json.get("details"), json.get("state"),
                json.get("largeImage"), json.get("largeText"),
                json.get("smallImage"), json.get("smallText"),
                json.getBool("timer"));
        }
    }
};

bool NativeBridge::isTransparent = false;