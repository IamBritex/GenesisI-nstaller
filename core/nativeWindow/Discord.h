#pragma once
#include <windows.h>
#include <string>
#include <ctime>
#include <vector>
#include <cstring>
#include "../discord/discord_game_sdk.h"
#include "Utils.h"

class DiscordManager {
private:
    struct IDiscordCore* core = nullptr;
    struct IDiscordActivityManager* activities = nullptr;
    bool isConnected = false;
    int64_t startTime = 0;

public:
    static DiscordManager& Get() {
        static DiscordManager instance;
        return instance;
    }

    void Initialize(const std::string& clientId) {
        if (clientId.empty()) return;

        // FIX: Inicializacion manual (memset) para evitar error de identificador no encontrado
        struct DiscordCreateParams params;
        std::memset(&params, 0, sizeof(params)); 
        
        DiscordClientId id = std::stoll(clientId);
        params.client_id = id;
        params.flags = DiscordCreateFlags_NoRequireDiscord;
        params.event_data = this; 
        
        // Creamos la instancia
        if (DiscordCreate(DISCORD_VERSION, &params, &core) == DiscordResult_Ok) {
            activities = core->get_activity_manager(core);
            isConnected = true;
            startTime = std::time(nullptr);
        }
    }

    void Update() {
        if (isConnected && core) {
            core->run_callbacks(core);
        }
    }

    void SetActivity(std::string details, std::string state, std::string largeImg, std::string largeText, std::string smallImg, std::string smallText, bool useTimer) {
        if (!isConnected || !activities) return;

        struct DiscordActivity activity;
        std::memset(&activity, 0, sizeof(activity));

        if (!details.empty()) strncpy_s(activity.details, details.c_str(), 127);
        if (!state.empty()) strncpy_s(activity.state, state.c_str(), 127);
        
        if (!largeImg.empty()) strncpy_s(activity.assets.large_image, largeImg.c_str(), 127);
        if (!largeText.empty()) strncpy_s(activity.assets.large_text, largeText.c_str(), 127);
        if (!smallImg.empty()) strncpy_s(activity.assets.small_image, smallImg.c_str(), 127);
        if (!smallText.empty()) strncpy_s(activity.assets.small_text, smallText.c_str(), 127);

        if (useTimer) {
            activity.timestamps.start = startTime;
        }

        // FIX: Lambda compatible con C puro
        activities->update_activity(activities, &activity, nullptr, [](void* data, enum EDiscordResult result) {});
    }

    void Shutdown() {
        if (core) {
            core->destroy(core);
            core = nullptr;
        }
        isConnected = false;
    }
};