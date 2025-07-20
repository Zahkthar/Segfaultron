#include "circus.h"

#include <stdio.h>
#include <stdlib.h>

static void initModule()
{
    puts("[INFO] - [Circus] - Module is initialized");
}

static void reloadModule()
{
    puts("[INFO] - [Circus] - Module is reloaded");
}

static void freeModule()
{
    puts("[INFO] - [Circus] - Module is freed");
}

static void on_message_create(struct discord *client, const struct discord_message *event)
{
    (void)client;
    printf("[INFO] - [Circus] - Creation : Message from %s: %s\n", event->author->username, event->content);

    // Repeat the user messages, test function
    if (event->author->bot) return;

    struct discord_create_message params = { 
        .content = event->content,
        .message_reference = !event->referenced_message
                ? NULL
                : &(struct discord_message_reference){
                    .message_id = event->referenced_message->id,
                    .channel_id = event->channel_id,
                    .guild_id = event->guild_id,
                },
        };

    discord_create_message(client, event->channel_id, &params, NULL);
}

static void on_message_update(struct discord *client, const struct discord_message *event)
{
    (void)client;
    (void)event;
}

static void on_message_delete(struct discord *client, const struct discord_message_delete *event)
{
    (void)client;
    (void)event;
}

SegfaultronModule *module_export() {
    SegfaultronModule *module = calloc(1, sizeof(SegfaultronModule));
    module->name = "Circus";

    module->initModuleFunction = initModule;
    module->reloadModuleFunction = reloadModule;
    module->freeModuleFunction = freeModule;

    module->onMessageCreateFunction = on_message_create;
    module->onMessageUpdateFunction = on_message_update;
    module->onMessageDeleteFunction = on_message_delete;
    return module;
}
