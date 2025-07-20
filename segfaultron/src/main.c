#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "concord/discord.h"
#include "concord/log.h"

#include "modules/modules.h"

u64snowflake g_app_id;

SegfaultronModuleList *modulesList = NULL;

void on_ready(struct discord *client, const struct discord_ready *event)
{
    (void)client;

    log_info("[INFO] Segfaultron connected as %s#%s!", event->user->username, event->user->discriminator);
    g_app_id = event->application->id;

    SegfaultronModules_loadModules(modulesList);
}

void on_interaction_create(struct discord *client, const struct discord_interaction *event)
{
    SegfaultronModules_on_interaction_create(modulesList, client, event);
}

void on_message_create(struct discord *client, const struct discord_message *event)
{
    SegfaultronModules_on_message_create(modulesList, client, event);
}

void on_message_update(struct discord *client, const struct discord_message *event)
{
    SegfaultronModules_on_message_update(modulesList, client, event);
}

void on_message_delete(struct discord *client, const struct discord_message_delete *event)
{
    SegfaultronModules_on_message_delete(modulesList, client, event);
}

int main(int argc, char *argv[])
{
    ccord_global_init();

    const char *config_file = (argc > 1) ? argv[1] : "config.json";
    struct discord *client = discord_config_init(config_file);

    assert(client != NULL && "[ERROR] Could not initialize Discord client");

    modulesList = SegfaultronModules_createList();

    discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT | DISCORD_GATEWAY_GUILD_MESSAGES | DISCORD_GATEWAY_GUILD_MEMBERS);

    discord_set_on_ready(client, &on_ready);

    discord_set_on_interaction_create(client, &on_interaction_create);
    discord_set_on_message_create(client, &on_message_create);
    discord_set_on_message_update(client, &on_message_update);
    discord_set_on_message_delete(client, &on_message_delete);

    discord_run(client);

    SegfaultronModules_freeList(modulesList);

    discord_cleanup(client);
    ccord_global_cleanup();
}
