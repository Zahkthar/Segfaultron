#include "circus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "concord/discord.h"
#include "cjson/cJSON.h"

#define SCORE_FILE "circus_scores.json"
#define CONFIG_FILE "circus_config.json"
#define DEFAULT_TIMEOUT 14400 // 4h in seconds
#define MAX_LINE 512

typedef struct
{
    u64snowflake channel_id;
    u64snowflake last_user_id;
    time_t last_msg_time;
    int timeout_seconds;
} TrackedChannel;

#define MAX_TRACKED_CHANNELS 32
TrackedChannel trackedChannels[MAX_TRACKED_CHANNELS];
size_t trackedChannelCount = 0;

static u64snowflake g_app_id;

static time_t currentUnixTime()
{
    return time(NULL);
}

static TrackedChannel *getTrackedChannel(u64snowflake channel_id)
{
    for (size_t i = 0; i < trackedChannelCount; ++i)
    {
        if (trackedChannels[i].channel_id == channel_id)
        {
            return &trackedChannels[i];
        }
    }

    return NULL;
}

static void initModule(struct discord *client, u64snowflake app_id)
{
    g_app_id = app_id;
    puts("[INFO] - [Circus] - Module is initialized");

    struct discord_create_global_application_command commands[4] =
    {
        {
            .name = "leaderboard",
            .description = "Affiche le classement du jeu des 4h",
        },
        {
            .name = "score",
            .description = "Affiche le score d'un utilisateur",
            .options = &(struct discord_application_command_options){
                .size = 1,
                .array = (struct discord_application_command_option[]){
                    {
                        .type = DISCORD_APPLICATION_OPTION_USER,
                        .name = "user",
                        .description = "Utilisateur à consulter",
                        .required = false,
                    }
                }
            }
        },
        {
            .name = "set",
            .description = "Définit un score pour un utilisateur",
            .options = &(struct discord_application_command_options){
                .size = 2,
                .array = (struct discord_application_command_option[]){
                    {
                        .type = DISCORD_APPLICATION_OPTION_USER,
                        .name = "user",
                        .description = "Utilisateur à modifier",
                        .required = true,
                    },
                    {
                        .type = DISCORD_APPLICATION_OPTION_INTEGER,
                        .name = "score",
                        .description = "Score à définir",
                        .required = true,
                    }
                }
            }
        },
        {
            .name = "config",
            .description = "Configure le timeout du salon",
            .options = &(struct discord_application_command_options){
                .size = 2,
                .array = (struct discord_application_command_option[]){
                    {
                        .type = DISCORD_APPLICATION_OPTION_CHANNEL,
                        .name = "channel",
                        .description = "Salon concerné",
                        .required = true,
                        .channel_types = &(struct integers){
                            .size = 1,
                            .array = (int[]){ DISCORD_CHANNEL_GUILD_TEXT },
                        },
                    },
                    {
                        .type = DISCORD_APPLICATION_OPTION_STRING,
                        .name = "timeout",
                        .description = "Délai d'inactivité (ex: 4h, 10m, 86400)",
                        .required = true,
                    }
                }
            }
        }
    };

    for (size_t i = 0; i < 4; ++i)
    {
        discord_create_global_application_command(client, app_id, &commands[i], NULL);
    }

    struct stat buffer;
    if (stat(CONFIG_FILE, &buffer) != 0)
    {
        FILE *file = fopen(CONFIG_FILE, "w");
        fprintf(file, "[]");
        fclose(file);
    }

    if (stat(SCORE_FILE, &buffer) != 0)
    {
        FILE *file = fopen(SCORE_FILE, "w");
        fprintf(file, "[]");
        fclose(file);
    }
}

static void freeModule()
{
    puts("[INFO] - [Circus] - Module is freed");
}

static void on_message_create(struct discord *client, const struct discord_message *event)
{
    (void)client;
    (void)event;

    if (event->author->bot) // Bots are not players
    {
        return;
    }

    printf("Un message a été créé\n");
}

static void handleLeaderboard(struct discord *client, const struct discord_interaction *event)
{
    (void)client;
    (void)event;
    printf("Leaderboard\n");
}

static void handleScore(struct discord *client, const struct discord_interaction *event)
{
    (void)client;
    (void)event;
    printf("Score\n");
}

static void handleSet(struct discord *client, const struct discord_interaction *event)
{
    (void)client;
    (void)event;
    printf("Set\n");
}

static void handleConfig(struct discord *client, const struct discord_interaction *event)
{
    (void)client;
    (void)event;
    printf("Config\n");
}

static void on_interaction_create(struct discord *client, const struct discord_interaction *event)
{
    if (!event->data || !event->data->name)
    {
        return;
    }

    if (strcmp(event->data->name, "leaderboard") == 0)
    {
        handleLeaderboard(client, event);
    }
    else if (strcmp(event->data->name, "score") == 0)
    {
        handleScore(client, event);
    }
    else if (strcmp(event->data->name, "set") == 0)
    {
        handleSet(client, event);
    }
    else if (strcmp(event->data->name, "config") == 0)
    {
        handleConfig(client, event);
    }
}

SegfaultronModule *module_export()
{
    SegfaultronModule *module = calloc(1, sizeof(SegfaultronModule));
    module->name = "Circus";

    module->initModuleFunction = initModule;
    module->freeModuleFunction = freeModule;

    module->onMessageCreateFunction = on_message_create;
    module->onInteractionCreateFunction = on_interaction_create;

    return module;
}
