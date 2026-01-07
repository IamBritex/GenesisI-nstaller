#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <fstream>
#include <filesystem>
#include <map>
#include <vector>
#include "Utils.h"

#pragma comment(lib, "ws2_32.lib")

namespace fs = std::filesystem;

class LocalServer
{
public:
    static int port;
    static bool running;

    static std::string GetMimeType(const std::string &ext)
    {
        static std::map<std::string, std::string> mimes = {
            {".html", "text/html"}, {".js", "application/javascript"}, {".css", "text/css"}, {".png", "image/png"}, {".jpg", "image/jpeg"}, {".gif", "image/gif"}, {".svg", "image/svg+xml"}, {".json", "application/json"}, {".wasm", "application/wasm"}, {".ico", "image/x-icon"}, {".mp3", "audio/mpeg"}, {".wav", "audio/wav"}};
        if (mimes.count(ext))
            return mimes[ext];
        return "application/octet-stream";
    }

    static void HandleClient(SOCKET clientSocket, std::wstring rootDir)
    {
        char buffer[4096];
        int bytesReceived = recv(clientSocket, buffer, 4096, 0);
        if (bytesReceived <= 0)
        {
            closesocket(clientSocket);
            return;
        }

        std::string request(buffer, bytesReceived);
        size_t methodEnd = request.find(' ');
        size_t pathEnd = request.find(' ', methodEnd + 1);
        if (methodEnd == std::string::npos || pathEnd == std::string::npos)
        {
            closesocket(clientSocket);
            return;
        }

        std::string path = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);
        if (path.find("..") != std::string::npos)
            path = "/";

        size_t queryPos = path.find('?');
        if (queryPos != std::string::npos)
            path = path.substr(0, queryPos);

        std::wstring wpath = std::wstring(path.begin(), path.end());
        if (wpath == L"/")
            wpath = L"/index.html";

        std::replace(wpath.begin(), wpath.end(), L'/', L'\\');
        std::wstring fullPath = rootDir + wpath;

        if (GetFileAttributesW(fullPath.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            std::string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(clientSocket, response.c_str(), (int)response.size(), 0);
        }
        else
        {
            std::ifstream file(fullPath, std::ios::binary);
            if (file)
            {
                file.seekg(0, std::ios::end);
                size_t fileSize = file.tellg();
                file.seekg(0, std::ios::beg);

                std::vector<char> fileData(fileSize);
                file.read(fileData.data(), fileSize);

                std::string ext = fs::path(fullPath).extension().string();
                std::string mime = GetMimeType(ext);

                std::string header = "HTTP/1.1 200 OK\r\n";
                header += "Content-Type: " + mime + "\r\n";
                header += "Access-Control-Allow-Origin: *\r\n";
                header += "Content-Length: " + std::to_string(fileSize) + "\r\n";
                header += "Connection: close\r\n\r\n";

                send(clientSocket, header.c_str(), (int)header.size(), 0);
                send(clientSocket, fileData.data(), (int)fileSize, 0);
            }
        }
        closesocket(clientSocket);
    }

    static void Start(int startPort, std::wstring rootDir)
    {
        port = startPort;
        running = true;
        std::thread([rootDir]()
                    {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);

            SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = INADDR_ANY;
            serverAddr.sin_port = htons(port);

            if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
                port++; 
                serverAddr.sin_port = htons(port);
                bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
            }

            listen(serverSocket, SOMAXCONN);

            while (running) {
                SOCKET client = accept(serverSocket, NULL, NULL);
                if (client != INVALID_SOCKET) {
                    std::thread(HandleClient, client, rootDir).detach();
                }
            }
            closesocket(serverSocket);
            WSACleanup(); })
            .detach();
    }
};

int LocalServer::port = 8080;
bool LocalServer::running = false;