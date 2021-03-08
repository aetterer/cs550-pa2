CC=gcc

# PN = peer node, IS = indexing server

BIN_DIR := bin
SRC_DIR := src
OBJ_DIR := obj
INC_DIR := inc

PN_BIN  := $(BIN_DIR)/peer_node
IS_BIN  := $(BIN_DIR)/indexing_server

PN_SRC  := $(SRC_DIR)/peer_node.c $(SRC_DIR)/client_server.c
IS_SRC  := $(SRC_DIR)/indexing_server.c $(SRC_DIR)/client_server.c
#SH_SRC  := $(SRC_DIR)/client_server.c

PN_OBJS := $(OBJ_DIR)/peer_node.o $(OBJ_DIR)/client_server.o
IS_OBJS := $(OBJ_DIR)/indexing_server.o $(OBJ_DIR)/client_server.o
#SH_OBJS := $(OBJ_DIR)/client_server.o

CPPFLAGS := -I$(INC_DIR)
CFLAGS   := -g -Wall

.PHONY: all clean

all: $(PN_BIN) $(IS_BIN)

$(PN_BIN): $(PN_OBJS) $(SH_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(PN_OBJS) $(SH_OBJ) -o $@

$(IS_BIN): $(IS_OBJS) $(SH_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(IS_OBJS) $(SH_OBJ) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rf $(BIN_DIR) $(OBJ_DIR)
