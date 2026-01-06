#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include "Utils.h"

struct AppConfig {
    std::wstring title = L"Genesis Engine";
    std::wstring appID = L"";
    std::wstring discordClientId = L"";
    std::wstring entryPoint = L"index.html";
    int width = 1280; int height = 720; 
    int minWidth = 800; int minHeight = 600;
    int fpsLimit = 300; 
    bool startMaximized = false; bool resizable = true; bool fullscreen = false;
    bool frame = true; bool alwaysOnTop = false; bool singleInstance = true; bool devTools = true;
};

class ConfigLoader {
public:
    static AppConfig LoadFromAppData(std::wstring appID) {
        AppConfig c;
        c.appID = appID;
        
        std::wstring configPath = Utils::GetAppDataPath(appID) + L"\\windowConfig.json";
        std::ifstream f(configPath);
        if (!f.is_open()) return c;
        
        std::stringstream buffer; buffer << f.rdbuf();
        Utils::MiniJSON json(buffer.str());

        std::wstring t = json.getW("title"); if(!t.empty()) c.title = t;
        std::wstring d = json.getW("discordClientId"); if(!d.empty()) c.discordClientId = d;
        std::wstring e = json.getW("entryPoint"); if(!e.empty()) c.entryPoint = e;
        
        int w = json.getInt("width"); if(w>0) c.width = w;
        int h = json.getInt("height"); if(h>0) c.height = h;
        
        c.startMaximized = json.getBool("startMaximized");
        c.resizable = json.getBool("resizable");
        c.frame = json.getBool("frame");
        c.devTools = json.getBool("devTools");
        
        return c;
    }
};