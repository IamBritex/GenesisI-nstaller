#ifndef LOCALHOST_H
#define LOCALHOST_H

#include <string>
#include <filesystem>

/**
 * @class LocalHost
 * @description Mapea rutas virtuales para que WebView2 pueda cargar archivos locales como un servidor.
 */
class LocalHost {
public:
    static std::wstring getFolderMapping(const std::string& appID) {
        char* path = nullptr;
        size_t size = 0;
        _dupenv_s(&path, &size, "APPDATA");
        std::string fullPath = std::string(path) + "\\" + appID + "\\assets";
        free(path);
        
        std::wstring wPath(fullPath.begin(), fullPath.end());
        return wPath;
    }
};

#endif