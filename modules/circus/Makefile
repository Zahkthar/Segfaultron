MODULE_NAME = circus

CXX = gcc
INCLUDE_MAIN = ../../include
HEADERS_LOCALISATION = include
CXXFLAGS = -Wall -Wextra -g -fPIC -I$(HEADERS_LOCALISATION) -I$(INCLUDE_MAIN)

LIB_LOCALISATION = lib
LDFLAGS = -L $(LIB_LOCALISATION) -shared -ldiscord

SRC_LOCALISATION = src
OBJ_LOCALISATION = obj

OUT_FILE = $(MODULE_NAME).so

SRCS = $(wildcard $(SRC_LOCALISATION)/*.c)
OBJS = $(patsubst $(SRC_LOCALISATION)/%.c,$(OBJ_LOCALISATION)/%.o,$(SRCS))

all: $(OUT_FILE)

$(OUT_FILE): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_LOCALISATION)/%.o: $(SRC_LOCALISATION)/%.c
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_LOCALISATION) $(OUT_FILE)

.PHONY: all clean
