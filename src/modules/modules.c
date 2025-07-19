#include "modules/modules.h"

#include <stdlib.h>

#include "modules/circus/circus.h"

/*
 * CREATE AND FREE LIST
 */

SegfaultronModuleList *SegfaultronModules_createList()
{
    SegfaultronModuleList *newList = malloc(sizeof(SegfaultronModuleList));

    if(newList == NULL)
    {
        return NULL;
    }

    newList->head = NULL;
    newList->tail = NULL;

    newList->length = 0;

    return newList;
}

static void SegfaultronModules_clearList(SegfaultronModuleList *modulesList)
{
    SegfaultronModule *current = modulesList->head;
    SegfaultronModule *tmp;

    // Destruction de tous les modules
    while (current != NULL)
    {
        tmp = current->next;
        current->freeModuleFunction();
        free(current);
        current = tmp;
    }
    
    modulesList->head = NULL;
    modulesList->tail = NULL;
}

void SegfaultronModules_freeList(SegfaultronModuleList *modulesList)
{
    SegfaultronModules_clearList(modulesList);
    free(modulesList);
}

/*
 * INSERT MODULE
 */

static void SegfaultronModules_insertModule(SegfaultronModuleList *modulesList, SegfaultronModule *newModule)
{
    if (modulesList->length == 0)
    {
        modulesList->head = newModule;
    }
    else
    {
        modulesList->tail->next = newModule;
    }

    modulesList->tail = newModule;
    newModule->next = NULL;
    modulesList->length += 1;
}

/*
 * LOAD AND RELOAD
 */

void SegfaultronModules_loadModules(SegfaultronModuleList *modulesList)
{
    SegfaultronModules_insertModule(modulesList, circusModule_export());
}

void SegfaultronModules_reloadModules(SegfaultronModuleList *modulesList)
{
    SegfaultronModule *currentModule = modulesList->head;

    while (currentModule != NULL)
    {
        currentModule->reloadModuleFunction();
        currentModule = currentModule->next;
    }
}

void SegfaultronModules_on_interaction_create(SegfaultronModuleList *modulesList, struct discord *client, const struct discord_interaction *event)
{
    SegfaultronModule *currentModule = modulesList->head;

    while(currentModule != NULL)
    {
        currentModule->onInteractionCreateFunction(client, event);
        currentModule = currentModule->next;
    }
}

void SegfaultronModules_on_message_create(SegfaultronModuleList *modulesList, struct discord *client, const struct discord_message *event)
{
    SegfaultronModule *currentModule = modulesList->head;

    while(currentModule != NULL)
    {
        currentModule->onMessageCreateFunction(client, event);
        currentModule = currentModule->next;
    }
}

void SegfaultronModules_on_message_update(SegfaultronModuleList *modulesList, struct discord *client, const struct discord_message *event)
{
    SegfaultronModule *currentModule = modulesList->head;

    while(currentModule != NULL)
    {
        currentModule->onMessageUpdateFunction(client, event);
        currentModule = currentModule->next;
    }
}

void SegfaultronModules_on_message_delete(SegfaultronModuleList *modulesList, struct discord *client, const struct discord_message_delete *event)
{
    SegfaultronModule *currentModule = modulesList->head;

    while(currentModule != NULL)
    {
        currentModule->onMessageDeleteFunction(client, event);
        currentModule = currentModule->next;
    }
}
