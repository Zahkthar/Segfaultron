CXX = gcc
CXXFLAGS = -Wall -Wextra -g# Mettre -O1 ou -O2 à la place de -g pour la version prod
HEADERS_LOCALISATION = include

LIB_LOCALISATION = lib
LDFLAGS = -ldiscord -lcurl -lpthread

BIN_LOCALISATION = bin
EXEC = Segfaultron

SRC_LOCALISATION = src
OBJS_LOCALISATION = obj

SRCS := $(shell find $(SRC_LOCALISATION) -type f -name "*.c")
OBJS := $(patsubst $(SRC_LOCALISATION)/%.c, $(OBJS_LOCALISATION)/%.o, $(SRCS))

all : $(BIN_LOCALISATION)/$(EXEC)

$(BIN_LOCALISATION)/$(EXEC) : $(OBJS)
	mkdir -p $(BIN_LOCALISATION)
	$(CXX) -L $(LIB_LOCALISATION) $^ -o $@ $(LDFLAGS)

$(OBJS_LOCALISATION)/%.o: $(SRC_LOCALISATION)/%.c
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I $(HEADERS_LOCALISATION) -c $< -o $@

clean:
	rm -rf $(OBJS_LOCALISATION)

mrproper: clean
	rm -f $(BIN_LOCALISATION)/$(EXEC) $(BIN_LOCALISATION)/*.log

.PHONY: all program clean mrproper