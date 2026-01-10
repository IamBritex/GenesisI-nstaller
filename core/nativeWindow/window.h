#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>
#include <dwmapi.h>
#include <string>
#include <stdio.h>
#include "../config.h"
#include "webView.h"
#include "fileSystem.h"

// Convierte Hex (#RRGGBB) a COLORREF
COLORREF HexToRGB(const std::string& hex) {
    if (hex.size() < 7 || hex[0] != '#') return RGB(0, 0, 0);
    int r, g, b;
    if (sscanf_s(hex.c_str(), "#%02x%02x%02x", &r, &g, &b) == 3) {
        return RGB(r, g, b);
    }
    return RGB(0,0,0);
}

class NativeWindow {
public:
    static WebViewInstance* webViewPtr;
    static WindowConfig currentConfig;
    static HBRUSH hBackgroundBrush;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_SIZE:
                if (webViewPtr) webViewPtr->Resize(hwnd);
                return 0;
            case WM_GETMINMAXINFO: {
                MINMAXINFO* mmi = (MINMAXINFO*)lParam;
                mmi->ptMinTrackSize.x = currentConfig.minWidth;
                mmi->ptMinTrackSize.y = currentConfig.minHeight;
                return 0;
            }
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                FillRect(hdc, &ps.rcPaint, hBackgroundBrush);
                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_DESTROY:
                if(hBackgroundBrush) DeleteObject(hBackgroundBrush);
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    static HWND Create(const WindowConfig& config, WebViewInstance& wv) {
        if (config.singleInstance) {
            CreateMutexA(NULL, TRUE, config.appID.c_str());
            if (GetLastError() == ERROR_ALREADY_EXISTS) {
                return NULL; 
            }
        }

        webViewPtr = &wv;
        currentConfig = config;
        hBackgroundBrush = CreateSolidBrush(HexToRGB(config.backgroundColor));

        HINSTANCE hInstance = GetModuleHandle(NULL);
        const wchar_t CLASS_NAME[] = L"GenesisEngineWindow";

        // CAMBIO: Usamos WNDCLASSEXW en lugar de WNDCLASSW para soportar hIconSm
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW); // IMPORTANTE: Tamaño de la estructura
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = hBackgroundBrush;
        
        // Intentar cargar el icono desde los assets desplegados
        if (!config.iconPath.empty()) {
            std::string iconP = FileSystem::getAssetPath(config.iconPath);
            HANDLE hIcon = LoadImageA(NULL, iconP.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
            if (hIcon) {
                wc.hIcon = (HICON)hIcon;
                wc.hIconSm = (HICON)hIcon; // Ahora sí es válido
            }
        }

        // CAMBIO: Usamos RegisterClassExW
        RegisterClassExW(&wc);

        DWORD style = WS_OVERLAPPEDWINDOW;
        if (!config.resizable) style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        if (!config.frame) style = WS_POPUP;

        int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
        int w = config.width, h = config.height;

        if (config.fullscreen) {
            style = WS_POPUP | WS_VISIBLE;
            x = 0; y = 0;
            w = GetSystemMetrics(SM_CXSCREEN);
            h = GetSystemMetrics(SM_CYSCREEN);
        }

        DWORD exStyle = config.alwaysOnTop ? WS_EX_TOPMOST : 0;

        HWND hwnd = CreateWindowExW(
            exStyle, CLASS_NAME, config.title.c_str(),
            style, x, y, w, h,
            NULL, NULL, hInstance, NULL
        );

        if (hwnd) {
            if (config.startMaximized && !config.fullscreen) ShowWindow(hwnd, SW_SHOWMAXIMIZED);
            else ShowWindow(hwnd, SW_SHOW);
            UpdateWindow(hwnd);
        }

        return hwnd;
    }
};

WebViewInstance* NativeWindow::webViewPtr = nullptr;
WindowConfig NativeWindow::currentConfig;
HBRUSH NativeWindow::hBackgroundBrush = NULL;

#endif