#include <windows.h>
#include <fstream>
#include <string>
#include "json.hpp"
#include "config.h"
#include "nativeWindow/window.h"
#include "nativeWindow/fileSystem.h"
#include "nativeWindow/webView.h"
#include "nativeWindow/APIs/Discord.h"

using json = nlohmann::json;

#ifndef GENESIS_APP_ID
#define GENESIS_APP_ID "com.genesis.engine"
#endif

WindowConfig loadConfigFromAppData()
{
    WindowConfig cfg;
    cfg.appID = GENESIS_APP_ID;
    cfg.title = L"Genesis Engine";
    cfg.backgroundColor = "#000000";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.minWidth = 800;
    cfg.minHeight = 600;
    cfg.resizable = true;
    cfg.frame = true;
    cfg.startMaximized = false;
    cfg.fullscreen = false;
    cfg.alwaysOnTop = false;
    cfg.singleInstance = true;
    cfg.build.entryPoint = "index.html";

    std::string configPath = FileSystem::getAppDataPath() + "\\assets\\windowConfig.json";
    std::ifstream file(configPath);

    if (file.is_open())
    {
        try
        {
            json j;
            file >> j;

            std::string t = j.value("title", "Genesis Engine");
            cfg.title = std::wstring(t.begin(), t.end());

            cfg.author = j.value("author", "");
            cfg.iconPath = j.value("icon", "");
            cfg.backgroundColor = j.value("backgroundColor", "#000000");

            cfg.width = j.value("width", 1280);
            cfg.height = j.value("height", 720);
            cfg.minWidth = j.value("minWidth", 800);
            cfg.minHeight = j.value("minHeight", 600);

            cfg.resizable = j.value("resizable", true);
            cfg.fullscreen = j.value("fullscreen", false);
            cfg.frame = j.value("frame", true);
            cfg.alwaysOnTop = j.value("alwaysOnTop", false);
            cfg.startMaximized = j.value("startMaximized", false);
            cfg.singleInstance = j.value("singleInstance", true);

            if (j.contains("build") && j["build"].contains("entryPoint"))
            {
                cfg.build.entryPoint = j["build"]["entryPoint"].get<std::string>();
            }
        }
        catch (...)
        {
        }
        file.close();
    }
    return cfg;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int show)
{
    FileSystem::setDllDirectory();
    WindowConfig config = loadConfigFromAppData();

    DiscordRPC discord;
    discord.init(123456789012345678);

    WebViewInstance wv;
    HWND hwnd = NativeWindow::Create(config, wv);

    if (hwnd)
        wv.Initialize(hwnd, config);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        discord.run();
    }
    return 0;
}