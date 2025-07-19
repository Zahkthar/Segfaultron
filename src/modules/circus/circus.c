#include "modules/circus/circus.h"

#include <stddef.h>
#include <stdlib.h>

#include <concord/discord.h>

#include "modules/modules.h"

void circusModule_init();
void circusModule_reload();
void circusModule_free();

void circusModule_onInteractionCreate(struct discord *client, const struct discord_interaction *event);
void circusModule_onMessageCreate(struct discord *client, const struct discord_message *event);
void circusModule_onMessageUpdate(struct discord *client, const struct discord_message *event);
void circusModule_onMessageDelete(struct discord *client, const struct discord_message_delete *event);

SegfaultronModule *circusModule_export()
{
    SegfaultronModule *circusModule = malloc(sizeof(SegfaultronModule));
    if (circusModule == NULL)
    {
        return NULL;
    }

    circusModule->name = "CircusModule";
    circusModule->initModuleFunction = circusModule_init;
    circusModule->reloadModuleFunction = circusModule_reload;
    circusModule->freeModuleFunction = circusModule_free;
    circusModule->onInteractionCreateFunction = circusModule_onInteractionCreate;
    circusModule->onMessageCreateFunction = circusModule_onMessageCreate;
    circusModule->onMessageUpdateFunction = circusModule_onMessageUpdate;
    circusModule->onMessageDeleteFunction = circusModule_onMessageDelete;
    circusModule->next = NULL;

    return circusModule;
}

void circusModule_init()
{
    
}

void circusModule_reload()
{
    circusModule_free();
    circusModule_init();
}

void circusModule_free()
{
    
}

void circusModule_onInteractionCreate(struct discord *client, const struct discord_interaction *event)
{
    (void)client;
    (void)event;
}

void circusModule_onMessageCreate(struct discord *client, const struct discord_message *event)
{
    (void)client;
    (void)event;

    puts("[CircusModule] - Message created");
}

void circusModule_onMessageUpdate(struct discord *client, const struct discord_message *event)
{
    (void)client;
    (void)event;

    puts("[CircusModule] - Message updated");
}

void circusModule_onMessageDelete(struct discord *client, const struct discord_message_delete *event)
{
    (void)client;
    (void)event;

    puts("[CircusModule] - Message deleted");
}