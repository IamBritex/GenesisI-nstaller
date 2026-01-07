#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <wrl.h>
#include <fstream>
#include <filesystem>

// Includes del proyecto con las rutas relativas correctas desde core/
#include "AppID.h"
#include "nativeWindow/Utils.h" // <--- RUTA CORREGIDA
#include "nativeWindow/Config.h"
#include "nativeWindow/Discord.h"
#include "nativeWindow/WebViewManager.h"
#include "nativeWindow/LoadingScreen.h"
#include "nativeWindow/LocalServer.h"

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace Microsoft::WRL;
namespace fs = std::filesystem;

AppConfig globalConfig;

void LoadEmbeddedDiscordDLL()
{
    std::wstring appDataPath = Utils::GetAppDataPath(GAME_APP_ID);
    std::wstring dllPath = appDataPath + L"\\discord_game_sdk.dll";

    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        HRSRC hRes = FindResourceW(NULL, L"DISCORD_DLL", RT_RCDATA);
        if (hRes)
        {
            HGLOBAL hData = LoadResource(NULL, hRes);
            DWORD size = SizeofResource(NULL, hRes);
            void *data = LockResource(hData);

            if (data && size > 0)
            {
                fs::create_directories(appDataPath);
                std::ofstream dllFile(dllPath, std::ios::binary);
                dllFile.write((char *)data, size);
                dllFile.close();
            }
        }
    }
    // Carga tardía para satisfacer al linker
    LoadLibraryW(dllPath.c_str());
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static ComPtr<ICoreWebView2Controller> controller;
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 30, NULL);
        break;
    case WM_TIMER:
        if (!WebViewManager::isLoaded)
        {
            LoadingScreen::rotationAngle = (LoadingScreen::rotationAngle + 15) % 360;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else
            KillTimer(hWnd, 1);
        break;
    case WM_PAINT:
        if (!WebViewManager::isLoaded)
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            LoadingScreen::Draw(hWnd, hdc);
            EndPaint(hWnd, &ps);
        }
        else
            return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    case WM_SIZE:
        if (controller)
        {
            RECT b;
            GetClientRect(hWnd, &b);
            controller->put_Bounds(b);
        }
        break;
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO *m = (MINMAXINFO *)lParam;
        m->ptMinTrackSize.x = globalConfig.minWidth;
        m->ptMinTrackSize.y = globalConfig.minHeight;
        return 0;
    }
    case WM_DESTROY:
        DiscordManager::Get().Shutdown();
        PostQuitMessage(0);
        break;
    case WM_USER + 1:
        controller = (ICoreWebView2Controller *)lParam;
        break;
    case WM_NCHITTEST:
    {
        LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
        if (hit == HTCLIENT && globalConfig.frame == false)
            return HTCAPTION;
        return hit;
    }
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    CoInitialize(NULL);
    LoadEmbeddedDiscordDLL();

    globalConfig = ConfigLoader::LoadFromAppData(GAME_APP_ID);

    // Iniciar servidor local
    std::wstring assetsPath = Utils::GetAppDataPath(GAME_APP_ID) + L"\\assets";
    LocalServer::Start(3000, assetsPath);

    CreateMutexW(NULL, TRUE, GAME_APP_ID);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;

    if (!globalConfig.discordClientId.empty())
    {
        DiscordManager::Get().Initialize(Utils::ToString(globalConfig.discordClientId));
    }

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"GenesisClass";

    std::wstring iconPath = Utils::GetAppDataPath(GAME_APP_ID) + L"\\assets\\icons\\icon.ico";
    HANDLE hIcon = LoadImageW(NULL, iconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (hIcon)
    {
        wc.hIcon = (HICON)hIcon;
        wc.hIconSm = (HICON)hIcon;
    }

    RegisterClassExW(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!globalConfig.resizable)
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    if (!globalConfig.frame)
        style = WS_POPUP;
    DWORD exStyle = globalConfig.alwaysOnTop ? WS_EX_TOPMOST : 0;

    RECT wr = {0, 0, globalConfig.width, globalConfig.height};
    AdjustWindowRectEx(&wr, style, FALSE, exStyle);

    HWND hWnd = CreateWindowExW(exStyle, L"GenesisClass", globalConfig.title.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInst, NULL);
    if (!hWnd)
        return 1;

    if (wc.hIcon)
    {
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)wc.hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)wc.hIcon);
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    WebViewManager::Initialize(hWnd, GAME_APP_ID, globalConfig);

    MSG msg;
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            DiscordManager::Get().Update();
            Sleep(5);
        }
    }

    DiscordManager::Get().Shutdown();
    return (int)msg.wParam;
}