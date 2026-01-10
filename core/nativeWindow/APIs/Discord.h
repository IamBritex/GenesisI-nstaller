#ifndef DISCORD_API_H
#define DISCORD_API_H

#include "../../discord/discord_game_sdk.h"
#include <cstring>

/**
 * @class DiscordRPC
 * @description Interfaz nativa para la comunicacion con el cliente de Discord.
 */
class DiscordRPC
{
private:
    IDiscordCore *core = nullptr;
    IDiscordActivityManager *activities = nullptr;

public:
    void init(int64_t clientId)
    {
        DiscordCreateParams params;
        DiscordCreateParamsSetDefault(&params);
        params.client_id = clientId;
        params.flags = DiscordCreateFlags_NoRequireDiscord;

        if (DiscordCreate(DISCORD_VERSION, &params, &core) == DiscordResult_Ok)
        {
            activities = core->get_activity_manager(core);
        }
    }

    void update(const char *details, const char *state)
    {
        if (!activities)
            return;
        DiscordActivity activity;
        memset(&activity, 0, sizeof(activity));
        strncpy(activity.details, details, 127);
        strncpy(activity.state, state, 127);
        activities->update_activity(activities, &activity, nullptr, nullptr);
    }

    void run()
    {
        if (core)
            core->run_callbacks(core);
    }
};

#endif