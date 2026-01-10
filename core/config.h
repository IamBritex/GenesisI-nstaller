#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

struct WindowConfig {
    std::wstring title;
    std::string appID;
    std::string author;
    std::string iconPath;
    std::string backgroundColor; // Hex code #000000

    int width;
    int height;
    int minWidth;
    int minHeight;

    bool resizable;
    bool fullscreen;
    bool frame;
    bool alwaysOnTop;
    bool startMaximized;
    bool singleInstance;
    bool hardwareAcceleration;
    bool devTools;

    struct Build {
        std::string entryPoint;
    } build;
};

#endif