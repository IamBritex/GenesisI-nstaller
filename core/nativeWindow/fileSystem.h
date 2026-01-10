#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <windows.h>

#ifndef GENESIS_APP_ID
#define GENESIS_APP_ID "com.genesis.engine"
#endif

class FileSystem {
public:
    static std::string getAppDataPath() {
        char* path = nullptr;
        size_t size = 0;
        if (_dupenv_s(&path, &size, "APPDATA") == 0 && path != nullptr) {
            std::string fullPath = std::string(path) + "\\" + GENESIS_APP_ID;
            free(path);
            return fullPath;
        }
        return "";
    }

    static std::string getAssetPath(const std::string& assetName) {
        return getAppDataPath() + "\\assets\\" + assetName;
    }

    static void setDllDirectory() {
        std::string path = getAppDataPath();
        if (!path.empty()) {
            std::wstring wPath(path.begin(), path.end());
            SetDllDirectoryW(wPath.c_str());
        }
    }
};

#endif