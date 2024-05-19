CC            := gcc
LDLIBS        := -pthread -lncurses
CFLAGS        := -Iheaders -DNDEBUG

BIN           := bin
OBJ           := obj
SRC           := sources

CLIENT        := client
SERVER        := server

TARGET_CLIENT := $(BIN)/$(CLIENT)
TARGET_SERVER := $(BIN)/$(SERVER)

SRCS_CLIENT   := $(shell find $(SRC)/client -name '*.c') $(shell find $(SRC) -maxdepth 1 -name '*.c')
SRCS_SERVER   := $(shell find $(SRC)/server -name '*.c') $(shell find $(SRC) -maxdepth 1 -name '*.c')

OBJS_CLIENT   := $(patsubst $(SRC)/%, $(OBJ)/%, $(SRCS_CLIENT:.c=.o))
OBJS_SERVER   := $(patsubst $(SRC)/%, $(OBJ)/%, $(SRCS_SERVER:.c=.o))

RESSOURCES    := res

.PHONY: all clean client server debug client_memory server_memory help

all: $(TARGET_SERVER) $(TARGET_CLIENT)

client: $(TARGET_CLIENT)

server: $(TARGET_SERVER)

debug: CFLAGS = -Iheaders
debug: all

client_memory: $(TARGET_CLIENT) $(OBJS_CLIENT)
	valgrind ./$(TARGET_CLIENT)

server_memory: $(TARGET_SERVER) $(OBJS_SERVER)
	valgrind ./$(TARGET_SERVER)

$(TARGET_CLIENT): $(OBJS_CLIENT)
	@mkdir -p $(RESSOURCES)/$(CLIENT)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDLIBS)

$(TARGET_SERVER): $(OBJS_SERVER)
	@mkdir -p $(RESSOURCES)/$(SERVER)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDLIBS)

# Generic rule to compile object files
$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) -r $(BIN) $(OBJ) $(RESSOURCES)

help:
	@echo "Usage:"
	@echo "    make [all]          Compilation du client et du serveur"
	@echo "    make debug          Compilation du client et du serveur en mode debug"
	@echo "    make client         Compilation du client"
	@echo "    make server         Compilation du serveur"
	@echo "    make clean          Supprime les .o et les executable"
	@echo "    make client_memory  Lance valgrind sur le client"
	@echo "    make server_memory  Lance valgrind sur le serveur"
	@echo "    make help           Affichage d'aide"
