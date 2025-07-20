#include "modules/modules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>

#define MODULES_DIR "modules"
#define EXPORT_SYMBOL "module_export"

typedef SegfaultronModule *(*ModuleExportFunc)();

static void insertModule(SegfaultronModuleList *modulesList, SegfaultronModule *module)
{
    module->next = NULL;

    if (modulesList->length == 0)
    {
        modulesList->head = module;
    }
    else
    {
        modulesList->tail->next = module;
    }
    modulesList->tail = module;
    modulesList->length++;
}

SegfaultronModuleList *SegfaultronModules_createList()
{
    SegfaultronModuleList *modulesList = calloc(1, sizeof(SegfaultronModuleList));
    return modulesList;
}

void SegfaultronModules_loadModules(SegfaultronModuleList *list)
{
    DIR *dir = opendir(MODULES_DIR);
    if (dir == NULL)
    {
        perror("[ERROR] - [ModuleLoader] Failed to open modules directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)))
    {
        if (!strstr(entry->d_name, ".so"))
        {
            continue;
        }

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", MODULES_DIR, entry->d_name);

        void *handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
        if (handle == NULL) 
        {
            fprintf(stderr, "[ERROR] - [ModuleLoader] dlopen failed: %s\n", dlerror());
            continue;
        }

        dlerror(); // clear previous error
        ModuleExportFunc moduleExportFunction = (ModuleExportFunc)dlsym(handle, EXPORT_SYMBOL);
        const char *err = dlerror();
        if (err != NULL) {
            fprintf(stderr, "[ERROR] - [ModuleLoader] dlsym failed: %s\n", err);
            dlclose(handle);
            continue;
        }

        SegfaultronModule *module = moduleExportFunction();
        if (module == NULL)
        {
            fprintf(stderr, "[ModuleLoader] module_export returned NULL\n");
            dlclose(handle);
            continue;
        }

        module->dlHandle = handle;
        insertModule(list, module);

        if (module->initModuleFunction)
        {
            module->initModuleFunction();
        }

        printf("[INFO] - [ModuleLoader] Loaded: %s\n", module->name);
    }

    closedir(dir);
}

void SegfaultronModules_reloadModules(SegfaultronModuleList *modulesList)
{
    for (SegfaultronModule *currentModule = modulesList->head; currentModule != NULL; currentModule = currentModule->next)
    {
        if (currentModule->reloadModuleFunction) {
            currentModule->reloadModuleFunction();
        }
    }
}

void SegfaultronModules_freeList(SegfaultronModuleList *modulesList) 
{
    SegfaultronModule *currentModule = modulesList->head;
    while (currentModule != NULL) {
        SegfaultronModule *tmp = currentModule->next;

        if (currentModule->freeModuleFunction)
        {
            currentModule->freeModuleFunction();
        }
        
        if (currentModule->dlHandle)
        {
            dlclose(currentModule->dlHandle);
        }

        free(currentModule);
        currentModule = tmp;
    }
    free(modulesList);
}

void SegfaultronModules_on_interaction_create(SegfaultronModuleList *modulesList, struct discord *client, const struct discord_interaction *event)
{
    for (SegfaultronModule *currentModule = modulesList->head; currentModule != NULL; currentModule = currentModule->next)
    {
        if (currentModule->onInteractionCreateFunction) {
            currentModule->onInteractionCreateFunction(client, event);
        }
    }
}

void SegfaultronModules_on_message_create(SegfaultronModuleList *modulesList, struct discord *client, const struct discord_message *event) 
{
    for (SegfaultronModule *currentModule = modulesList->head; currentModule != NULL; currentModule = currentModule->next)
    {
        if (currentModule->onMessageCreateFunction) {
            currentModule->onMessageCreateFunction(client, event);
        }
    }
}

void SegfaultronModules_on_message_update(SegfaultronModuleList *modulesList, struct discord *client, const struct discord_message *event)
{
    for (SegfaultronModule *currentModule = modulesList->head; currentModule != NULL; currentModule = currentModule->next)
    {
        if (currentModule->onMessageUpdateFunction) {
            currentModule->onMessageUpdateFunction(client, event);
        }
    }
}

void SegfaultronModules_on_message_delete(SegfaultronModuleList *modulesList, struct discord *client, const struct discord_message_delete *event)
{
    for (SegfaultronModule *currentModule = modulesList->head; currentModule != NULL; currentModule = currentModule->next)
    {
        if (currentModule->onMessageDeleteFunction) {
            currentModule->onMessageDeleteFunction(client, event);
        }
    }
}
