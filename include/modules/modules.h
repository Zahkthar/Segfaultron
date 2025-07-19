#ifndef SEGFAULTRON_MODULES_H_INCLUDED
#define SEGFAULTRON_MODULES_H_INCLUDED

#include "concord/discord.h"

typedef struct SegfaultronModule
{
    // Attributs
    char *name;
    
    // Fonctions
    void (*initModuleFunction)  ();
    void (*reloadModuleFunction)();
    void (*freeModuleFunction)  ();

    void (*onInteractionCreateFunction)(struct discord *client, const struct discord_interaction *event);
    void (*onMessageCreateFunction)(struct discord *client, const struct discord_message *event);
    void (*onMessageUpdateFunction)(struct discord *client, const struct discord_message *event);
    void (*onMessageDeleteFunction)(struct discord *client, const struct discord_message_delete *event);

    struct SegfaultronModule *next;
} SegfaultronModule;

typedef struct SegfaultronModuleList
{
    struct SegfaultronModule *head;
    struct SegfaultronModule *tail;
    size_t length;
} SegfaultronModuleList;

/*
 * CREATE AND FREE LIST
 */

SegfaultronModuleList *SegfaultronModules_createList();
void SegfaultronModules_freeList(SegfaultronModuleList *modulesList);

/*
 * LOAD AND RELOAD
 */

void SegfaultronModules_loadModules(SegfaultronModuleList *modulesList);
void SegfaultronModules_reloadModules(SegfaultronModuleList *modulesList);

/*
 * MODULE LOGIC
 */

void SegfaultronModules_on_interaction_create(SegfaultronModuleList *modulesList, struct discord *client, const struct discord_interaction *event);
void SegfaultronModules_on_message_create    (SegfaultronModuleList *modulesList, struct discord *client, const struct discord_message *event);
void SegfaultronModules_on_message_update    (SegfaultronModuleList *modulesList, struct discord *client, const struct discord_message *event);
void SegfaultronModules_on_message_delete    (SegfaultronModuleList *modulesList, struct discord *client, const struct discord_message_delete *event);

#endif // SEGFAULTRON_MODULES_H_INCLUDED