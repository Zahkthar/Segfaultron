#ifndef SEGFAULTRON_MODULES_H_INCLUDED
#define SEGFAULTRON_MODULES_H_INCLUDED

#include <stddef.h>
#include <dlfcn.h>

#include "concord/discord.h"

typedef struct SegfaultronModule {
    char *name;

    void (*initModuleFunction)();
    void (*reloadModuleFunction)();
    void (*freeModuleFunction)();

    // Module logic functions
    void (*onInteractionCreateFunction)(struct discord *, const struct discord_interaction *);
    void (*onMessageCreateFunction)(struct discord *, const struct discord_message *);
    void (*onMessageUpdateFunction)(struct discord *, const struct discord_message *);
    void (*onMessageDeleteFunction)(struct discord *, const struct discord_message_delete *);

    void *dlHandle;

    struct SegfaultronModule *next;
} SegfaultronModule;

typedef struct SegfaultronModuleList {
    SegfaultronModule *head;
    SegfaultronModule *tail;
    size_t length;
} SegfaultronModuleList;

SegfaultronModuleList *SegfaultronModules_createList();

void SegfaultronModules_freeList     (SegfaultronModuleList *list);
void SegfaultronModules_loadModules  (SegfaultronModuleList *list);
void SegfaultronModules_reloadModules(SegfaultronModuleList *list);

void SegfaultronModules_on_interaction_create(SegfaultronModuleList *, struct discord *, const struct discord_interaction *);
void SegfaultronModules_on_message_create    (SegfaultronModuleList *, struct discord *, const struct discord_message *);
void SegfaultronModules_on_message_update    (SegfaultronModuleList *, struct discord *, const struct discord_message *);
void SegfaultronModules_on_message_delete    (SegfaultronModuleList *, struct discord *, const struct discord_message_delete *);

#endif // SEGFAULTRON_MODULES_H_INCLUDED
