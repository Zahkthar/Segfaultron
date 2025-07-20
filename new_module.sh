#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <module_name>"
    exit 1
fi

MODULE=$1
BASE_DIR="modules/$MODULE"

mkdir -p "$BASE_DIR/src" "$BASE_DIR/include" "$BASE_DIR/obj"

# module c file
cat > "$BASE_DIR/src/$MODULE.c" << EOF
#include "$MODULE.h"

#include <stdio.h>
#include <stdlib.h>

static void initModule(struct discord *client, u64snowflake app_id)
{
    puts("[INFO] - [$MODULE] Module initialized");
}

static void freeModule()
{
    puts("[INFO] - [$MODULE] Module freed");
}

SegfaultronModule *module_export()
{
    SegfaultronModule *module = calloc(1, sizeof(SegfaultronModule));
    module->name = "$MODULE";
    module->initModuleFunction = initModule;
    module->freeModuleFunction = freeModule;
    return module;
}

EOF

# module header
cat > "$BASE_DIR/include/$MODULE.h" << EOF
#ifndef ${MODULE^^}_MODULE_H_INCLUDED
#define ${MODULE^^}_MODULE_H_INCLUDED

#include "modules/modules.h"

SegfaultronModule *module_export();

#endif // ${MODULE^^}_MODULE_H_INCLUDED
EOF

# Makefile
cat > "$BASE_DIR/Makefile" << EOF
MODULE_NAME = $MODULE

CXX = gcc
INCLUDE_MAIN = ../../segfaultron/include
HEADERS_LOCALISATION = include
CXXFLAGS = -Wall -Wextra -g -fPIC -I\$(HEADERS_LOCALISATION) -I\$(INCLUDE_MAIN)

LIB_LOCALISATION = lib
LDFLAGS = -L \$(LIB_LOCALISATION) -shared -ldiscord

SRC_LOCALISATION = src
OBJ_LOCALISATION = obj

BIN_LOCALISATION = bin
OUT_FILE = \$(MODULE_NAME).so

SRCS = \$(wildcard \$(SRC_LOCALISATION)/*.c)
OBJS = \$(patsubst \$(SRC_LOCALISATION)/%.c,\$(OBJ_LOCALISATION)/%.o,\$(SRCS))

all: \$(BIN_LOCALISATION)/\$(OUT_FILE)

\$(BIN_LOCALISATION)/\$(OUT_FILE): \$(OBJS)
	mkdir -p \$(BIN_LOCALISATION)
	\$(CXX) \$^ -o \$@ \$(LDFLAGS)

\$(OBJ_LOCALISATION)/%.o: \$(SRC_LOCALISATION)/%.c
	mkdir -p \$(dir \$@)
	\$(CXX) \$(CXXFLAGS) -c \$< -o \$@

clean:
	rm -rf \$(OBJ_LOCALISATION) \$(BIN_LOCALISATION)

.PHONY: all clean

EOF

echo "[INFO] Module '$MODULE' skeleton created in $BASE_DIR"
