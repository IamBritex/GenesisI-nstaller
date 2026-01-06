#pragma once
#include <windows.h>
#include <string>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <wincrypt.h>
#include <map>
#include <vector>

#pragma comment(lib, "Crypt32.lib")

namespace fs = std::filesystem;

namespace Utils {
    inline std::wstring ToWString(const std::string& str) {
        if (str.empty()) return std::wstring();
        int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstr(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size);
        return wstr;
    }
    
    inline std::string ToString(const std::wstring& wstr) {
        if (wstr.empty()) return std::string();
        int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string str(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size, NULL, NULL);
        return str;
    }

    inline std::string Base64Encode(const std::vector<BYTE>& data) {
        DWORD size = 0;
        if (CryptBinaryToStringA(data.data(), (DWORD)data.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &size)) {
            std::string str(size, '\0');
            if (CryptBinaryToStringA(data.data(), (DWORD)data.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &str[0], &size)) return str;
        }
        return "";
    }

    inline std::vector<BYTE> Base64Decode(const std::string& b64) {
        DWORD size = 0;
        if (CryptStringToBinaryA(b64.c_str(), 0, CRYPT_STRING_BASE64_ANY, NULL, &size, NULL, NULL)) {
            std::vector<BYTE> data(size);
            if (CryptStringToBinaryA(b64.c_str(), 0, CRYPT_STRING_BASE64_ANY, data.data(), &size, NULL, NULL)) return data;
        }
        return {};
    }

    inline std::wstring GetAppDataPath(std::wstring appID) {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path))) return std::wstring(path) + L"\\" + appID;
        return L"";
    }

    inline bool IsSafePath(const fs::path& root, const fs::path& child, fs::path& result) {
        try {
            fs::path absRoot = fs::canonical(root);
            fs::path absChild = fs::weakly_canonical(root / child);
            if (absChild.wstring().find(absRoot.wstring()) == 0) { result = absChild; return true; }
        } catch(...) {}
        return false;
    }

    // --- MINIJSON v2.0 (ROBUSTO) ---
    class MiniJSON {
        std::map<std::string, std::string> data;
        
        // Limpia comillas y espacios de una cadena raw
        std::string Clean(std::string s) {
            size_t first = s.find_first_not_of(" \t\n\r\"");
            if (std::string::npos == first) return "";
            size_t last = s.find_last_not_of(" \t\n\r\"");
            return s.substr(first, (last - first + 1));
        }

    public:
        MiniJSON(std::string json) {
            // Parseador simple basado en busqueda de claves
            // Asumimos estructura plana: { "key": value, "key2": "value2" }
            bool insideString = false;
            std::string buffer = "";
            std::string key = "";
            
            for (size_t i = 0; i < json.length(); i++) {
                char c = json[i];
                
                if (c == '"' && (i == 0 || json[i-1] != '\\')) {
                    insideString = !insideString;
                    if(!insideString) buffer += '"'; // Guardar comilla de cierre
                    else buffer += '"'; // Guardar comilla de apertura
                    continue;
                }

                if (insideString) {
                    buffer += c;
                } else {
                    if (c == '{' || c == '}' || c == '\n' || c == '\r' || c == '\t') continue;
                    
                    if (c == ':') {
                        key = Clean(buffer);
                        buffer = "";
                    } 
                    else if (c == ',') {
                        if (!key.empty()) data[key] = Clean(buffer);
                        key = ""; buffer = "";
                    } 
                    else {
                        buffer += c;
                    }
                }
            }
            // Guardar el ultimo valor si no hubo coma final
            if (!key.empty() && !buffer.empty()) data[key] = Clean(buffer);
        }

        std::string get(std::string key) { return data.count(key) ? data[key] : ""; }
        std::wstring getW(std::string key) { return ToWString(get(key)); }
        
        int getInt(std::string key) { 
            std::string val = get(key);
            try { return val.empty() ? 0 : std::stoi(val); } catch(...) { return 0; } 
        }
        
        bool getBool(std::string key) { 
            std::string val = get(key);
            return (val == "true" || val == "1"); 
        }
    };
}