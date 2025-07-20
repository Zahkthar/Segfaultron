#include "circus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "concord/discord.h"
#include "cjson/cJSON.h"

#define JD4H_FILE "jd4h.json"

typedef struct {
    u64snowflake user_id;
    int points;
} UserScore;

typedef struct {
    u64snowflake channel_id;
    u64snowflake last_user_id;
    time_t last_msg_time;
    int timeout_seconds;

    UserScore *scores;
    size_t score_count;
    size_t score_capacity;
} TrackedChannel;

typedef struct {
    TrackedChannel *trackedChannel;
    UserScore *userScore;
} UserInfos;

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

UserInfos getUserInfos(u64snowflake channelID, u64snowflake userID)
{
    TrackedChannel *channel = NULL;
    UserScore    *userScore = NULL;
    for(size_t i = 0; i < trackedChannelCount; ++i)
    {
        if(trackedChannels[i].channel_id == channelID)
        {
            channel = &trackedChannels[i];
            for(size_t j = 0; j < trackedChannels[i].score_count; ++j)
            {
                if(trackedChannels[i].scores[j].user_id == userID)
                {
                    userScore = &trackedChannels[i].scores[j];
                    break;
                }
            }
            break;
        }
    }

    return (UserInfos){channel, userScore};
}

void setUserScore(u64snowflake channelID, u64snowflake userID, int newPoints)
{
    UserInfos userInfos = getUserInfos(channelID, userID);
    
    TrackedChannel *trackedChannel = userInfos.trackedChannel;
    UserScore      *userScore      = userInfos.userScore;

    if(trackedChannel == NULL)
    {
        printf("[WARN] - [Circus] - Le jeu des 4h n'est pas configuré dans le salon <#%" PRIu64 "> (Tentative de set les points de l'utilisateur <@%" PRIu64 "> à %d)",
               channelID, userID, newPoints
              );
    }

    if(userScore == NULL)
    {
        if (trackedChannel->score_count >= trackedChannel->score_capacity)
        {
            size_t new_capacity = trackedChannel->score_capacity == 0 ? 4 : trackedChannel->score_capacity * 2;
            UserScore *new_scores = realloc(trackedChannel->scores, new_capacity * sizeof(UserScore));
            if (new_scores == NULL)
            {
                return;
            }
            trackedChannel->scores = new_scores;
            trackedChannel->score_capacity = new_capacity;
        }

        trackedChannel->scores[trackedChannel->score_count].user_id = userID;
        trackedChannel->scores[trackedChannel->score_count].points  = newPoints;
        trackedChannel->score_count++;
    }
    else
    {
        userScore->points = newPoints;
    }
}

static void loadTrackedChannels() {
    FILE *file = fopen(JD4H_FILE, "r");
    if (file == NULL)
    {
        return;
    }

    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    rewind(file);

    char *data = malloc(len + 1);
    fread(data, 1, len, file);
    data[len] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(data);
    free(data);
    if (root == NULL)
    {
        return;
    }

    cJSON *array = cJSON_GetObjectItem(root, "trackedChannels");
    if (cJSON_IsArray(array) == false)
    {
        cJSON_Delete(root);
        return;
    }

    trackedChannelCount = 0;

    cJSON *entry = NULL;
    cJSON_ArrayForEach(entry, array)
    {
        if (trackedChannelCount >= MAX_TRACKED_CHANNELS)
        {
            break;
        }

        cJSON *j_channel_id      = cJSON_GetObjectItem(entry, "channel_id");
        cJSON *j_last_user_id    = cJSON_GetObjectItem(entry, "last_user_id");
        cJSON *j_last_msg_time   = cJSON_GetObjectItem(entry, "last_msg_time");
        cJSON *j_timeout_seconds = cJSON_GetObjectItem(entry, "timeout_seconds");

        if (!cJSON_IsString(j_channel_id) || !cJSON_IsString(j_last_user_id) || !cJSON_IsString(j_last_msg_time) || !cJSON_IsString(j_timeout_seconds))
        {
            continue;
        }

        TrackedChannel channel = {
            .channel_id      = strtoull(j_channel_id->valuestring, NULL, 10),
            .last_user_id    = strtoull(j_last_user_id->valuestring, NULL, 10),
            .last_msg_time   = (time_t)strtoll(j_last_msg_time->valuestring, NULL, 10),
            .timeout_seconds = atoi(j_timeout_seconds->valuestring),
            .scores = NULL,
            .score_count = 0,
            .score_capacity = 0
        };

        cJSON *points_array = cJSON_GetObjectItem(entry, "points");
        if (cJSON_IsArray(points_array))
        {
            cJSON *point_entry = NULL;
            cJSON_ArrayForEach(point_entry, points_array) {
                cJSON *userID = cJSON_GetObjectItem(point_entry, "user_id");
                cJSON *userPoints  = cJSON_GetObjectItem(point_entry, "points");

                if (!cJSON_IsString(userID) || !cJSON_IsNumber(userPoints))
                {
                    continue;
                }

                if (channel.score_count >= channel.score_capacity)
                {
                    size_t new_capacity = channel.score_capacity == 0 ? 4 : channel.score_capacity * 2;
                    UserScore *new_scores = realloc(channel.scores, new_capacity * sizeof(UserScore));
                    if (new_scores == NULL)
                    {
                        break;
                    }
                    channel.scores = new_scores;
                    channel.score_capacity = new_capacity;
                }

                channel.scores[channel.score_count].user_id = strtoull(userID->valuestring, NULL, 10);
                channel.scores[channel.score_count].points  = userPoints->valueint;
                channel.score_count++;
            }
        }

        trackedChannels[trackedChannelCount++] = channel;
    }

    cJSON_Delete(root);
}

static void saveTrackedChannels()
{
    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();

    for (size_t i = 0; i < trackedChannelCount; ++i)
    {
        TrackedChannel *channel = &trackedChannels[i];
        cJSON *obj = cJSON_CreateObject();

        char channel_id_str[32];
        char last_user_id_str[32];
        char last_msg_time_str[32];
        char timeout_seconds_str[16];

        snprintf(channel_id_str, sizeof(channel_id_str), "%" PRIu64, channel->channel_id);
        snprintf(last_user_id_str, sizeof(last_user_id_str), "%" PRIu64, channel->last_user_id);
        snprintf(last_msg_time_str, sizeof(last_msg_time_str), "%ld", channel->last_msg_time);
        snprintf(timeout_seconds_str, sizeof(timeout_seconds_str), "%d", channel->timeout_seconds);

        cJSON_AddStringToObject(obj, "channel_id", channel_id_str);
        cJSON_AddStringToObject(obj, "last_user_id", last_user_id_str);
        cJSON_AddStringToObject(obj, "last_msg_time", last_msg_time_str);
        cJSON_AddStringToObject(obj, "timeout_seconds", timeout_seconds_str);

        cJSON *points_array = cJSON_CreateArray();
        for (size_t j = 0; j < channel->score_count; ++j)
        {
            UserScore *userScore = &channel->scores[j];

            char userIDstr[32];
            snprintf(userIDstr, sizeof(userIDstr), "%" PRIu64, userScore->user_id);

            cJSON *score_obj = cJSON_CreateObject();
            cJSON_AddStringToObject(score_obj, "user_id", userIDstr);
            cJSON_AddNumberToObject(score_obj, "points", userScore->points);
            cJSON_AddItemToArray(points_array, score_obj);
        }

        cJSON_AddItemToObject(obj, "points", points_array);
        cJSON_AddItemToArray(array, obj);
    }

    cJSON_AddItemToObject(root, "trackedChannels", array);

    char *jsonString = cJSON_Print(root);
    if (jsonString)
    {
        FILE *file = fopen(JD4H_FILE, "w");
        if (file)
        {
            fputs(jsonString, file);
            fclose(file);
        }
        free(jsonString);
    }

    cJSON_Delete(root);
}

static int parseTimeoutString(const char *str)
{
    int len = strlen(str);
    if (len == 0)
    {
        return -1;
    }

    char unit = str[len - 1];
    char numberStr[16] = {0};
    strncpy(numberStr, str, len - (isalpha(unit) ? 1 : 0));

    int number = atoi(numberStr);
    if (number <= 0)
    {
        return -1;
    }

    switch (unit)
    {
        case 's': return number;
        case 'm': return number * 60;
        case 'h': return number * 3600;
        case 'd': return number * 86400;
        default:  return atoi(str);
    }
}

static void initModule(struct discord *client, u64snowflake app_id)
{
    g_app_id = app_id;
    puts("[INFO] - [Circus] - Module is initialized");
    
    //struct discord_create_global_application_command commands[4] =
    struct discord_create_guild_application_command commands[4] =
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
        //discord_create_global_application_command(client, app_id, &commands[i], NULL);
        discord_create_guild_application_command(client, app_id, 1147874619425038460, &commands[i], NULL);
    }

    struct stat buffer;
    if (stat(JD4H_FILE, &buffer) != 0)
    {
        FILE *file = fopen(JD4H_FILE, "w");
        fprintf(file, "{}");
        fclose(file);
    }

    loadTrackedChannels();
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
    
    struct discord_interaction_response response =
    {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data)
        {
            .content = "Voici la commande leaderboard (en construction 🛠️)",
        }
    };

    discord_create_interaction_response(client, event->id, event->token, &response, NULL);
}

static void handleScore(struct discord *client, const struct discord_interaction *event)
{
    (void)client;    

    if (!event->data)
    {
        return;
    }

    u64snowflake userID = event->member->user->id;

    if (event->data->options && event->data->options->size > 0) 
    {
        for (int i = 0; i < event->data->options->size; ++i)
        {
            char *name = event->data->options->array[i].name;
            char *value = event->data->options->array[i].value;

            if (0 == strcmp(name, "user"))
            {
                sscanf(value, "%" SCNu64, &userID);
            }
        }
    }

    UserInfos userInfos = getUserInfos(event->channel_id, userID);

    if(userInfos.trackedChannel == NULL)
    {
        struct discord_interaction_response response =
        {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data = &(struct discord_interaction_callback_data)
            {
                .content = "Le jeu des 4h n'est pas configuré dans ce salon",
            }
        };
        discord_create_interaction_response(client, event->id, event->token, &response, NULL);
        return;
    }

    int score = (userInfos.userScore == NULL) ? 0 : userInfos.userScore->points;

    char responseStr[512];
    snprintf(responseStr, sizeof(responseStr), "Le joueur <@%" PRIu64 "> a %d point%s.", userID, score, score > 1 ? "s" : "");
    struct discord_interaction_response response =
    {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data)
        {
            .content = responseStr,
        }
    };

    discord_create_interaction_response(client, event->id, event->token, &response, NULL);
}

static void handleSet(struct discord *client, const struct discord_interaction *event)
{
    (void)client;    

    if (!event->data || !event->data->options)
    {
        return;
    }

    u64snowflake userID = 0;
    int score = 0;
    u64snowflake channelID = event->channel_id;

    for (int i = 0; i < event->data->options->size; ++i) {
        char *name = event->data->options->array[i].name;
        char *value = event->data->options->array[i].value;

        if (0 == strcmp(name, "user"))
        {
            sscanf(value, "%" SCNu64, &userID);
        }
        else if (0 == strcmp(name, "score"))
        {
            score = strtol(value, NULL, 10);
        }
    }

    UserInfos userInfos = getUserInfos(channelID, userID);

    if(userInfos.trackedChannel == NULL)
    {
        struct discord_interaction_response response =
        {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data = &(struct discord_interaction_callback_data)
            {
                .content = "Le jeu des 4h n'est pas configuré dans ce salon",
            }
        };
        discord_create_interaction_response(client, event->id, event->token, &response, NULL);
        return;
    }

    setUserScore(channelID, userID, score);

    saveTrackedChannels();

    char responseStr[512];
    snprintf(responseStr, sizeof(responseStr), "Le joueur <@%" PRIu64 "> a maintenant %d point%s.", userID, score, score > 1 ? "s" : "");
    struct discord_interaction_response response =
    {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data)
        {
            .content = responseStr
        }
    };

    discord_create_interaction_response(client, event->id, event->token, &response, NULL);
}

static void handleConfig(struct discord *client, const struct discord_interaction *event)
{
    (void)client;

    if (!event->data || !event->data->options)
    {
        return;
    }

    time_t now = currentUnixTime();

    u64snowflake channelID = 0;
    char *timeoutStr = "4h";

    for (int i = 0; i < event->data->options->size; ++i) {
        char *name = event->data->options->array[i].name;
        char *value = event->data->options->array[i].value;

        if (0 == strcmp(name, "channel"))
        {
            sscanf(value, "%" SCNu64, &channelID);
        }
        else if (0 == strcmp(name, "timeout"))
        {
            timeoutStr = value;
        }
    }

    int timeout = parseTimeoutString(timeoutStr);
    if (timeout <= 0) 
    {
        struct discord_interaction_response err =
        {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data = &(struct discord_interaction_callback_data)
            {
                .content = "⛔ Format du délai invalide. Ex: `4h`, `10m`, `86400`..."
            }
        };
        discord_create_interaction_response(client, event->id, event->token, &err, NULL);
        return;
    }

    TrackedChannel *trackedChannel = getTrackedChannel(channelID);
    if(trackedChannel == NULL)
    {
        trackedChannels[trackedChannelCount++] = (TrackedChannel){channelID, 0, now, timeout};
    }
    else
    {
        trackedChannel->timeout_seconds = timeout;
    }

    saveTrackedChannels();

    char responseStr[256];
    snprintf(responseStr, sizeof(responseStr), "✅ Timeout configuré à %d seconde%s pour <#%" PRIu64 ">.", timeout, timeout > 1 ? "s" : "", channelID);

    struct discord_interaction_response response =
    {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data)
        {
            .content = responseStr
        }
    };

    discord_create_interaction_response(client, event->id, event->token, &response, NULL);
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
