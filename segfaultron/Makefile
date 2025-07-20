CXX = gcc
HEADERS_LOCALISATION = include
CXXFLAGS = -Wall -Wextra -g -I $(HEADERS_LOCALISATION)# Mettre -O1 ou -O2 à la place de -g pour la version prod

LIB_LOCALISATION = lib
LDFLAGS = -L $(LIB_LOCALISATION) -Wl,-rpath,/usr/local/lib -ldiscord -lcurl -lpthread

BIN_LOCALISATION = bin
EXEC = Segfaultron

SRC_LOCALISATION = src
OBJ_LOCALISATION = obj

SRCS := $(shell find $(SRC_LOCALISATION) -type f -name "*.c")
OBJS := $(patsubst $(SRC_LOCALISATION)/%.c, $(OBJ_LOCALISATION)/%.o, $(SRCS))

all : $(BIN_LOCALISATION)/$(EXEC)

$(BIN_LOCALISATION)/$(EXEC) : $(OBJS)
	mkdir -p $(BIN_LOCALISATION)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_LOCALISATION)/%.o: $(SRC_LOCALISATION)/%.c
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_LOCALISATION)
	rm -f $(BIN_LOCALISATION)/$(EXEC) $(BIN_LOCALISATION)/*.log
	
.PHONY: all program clean mrproper