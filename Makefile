CC := gcc
CFLAGS := -Iheaders -lncurses
SRC := sources
CLIENT_SRCDIR := sources/client
SERVER_SRCDIR := sources/server
CLIENT_OBJDIR := obj/client
SERVER_OBJDIR := obj/server
CLIENT_EXECUTABLE := client
SERVER_EXECUTABLE := server

CLIENT_SOURCES := $(wildcard $(CLIENT_SRCDIR)/*.c $(SRC)/*.c)
SERVER_SOURCES := $(wildcard $(SERVER_SRCDIR)/*.c $(SRC)/*.c)
CLIENT_OBJECTS := $(patsubst $(CLIENT_SRCDIR)/%.c,$(CLIENT_OBJDIR)/%.o,$(CLIENT_SOURCES))
SERVER_OBJECTS := $(patsubst $(SERVER_SRCDIR)/%.c,$(SERVER_OBJDIR)/%.o,$(SERVER_SOURCES))

.PHONY: all clean client server

all: client server

client: $(CLIENT_EXECUTABLE)

server: $(SERVER_EXECUTABLE)

$(CLIENT_EXECUTABLE): $(CLIENT_OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

$(SERVER_EXECUTABLE): $(SERVER_OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

$(CLIENT_OBJDIR)/%.o: $(CLIENT_SRCDIR)/%.c
	@mkdir -p $(CLIENT_OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(SERVER_OBJDIR)/%.o: $(SERVER_SRCDIR)/%.c
	@mkdir -p $(SERVER_OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf $(CLIENT_OBJDIR)/*.o $(SERVER_OBJDIR)/*.o $(CLIENT_EXECUTABLE) $(SERVER_EXECUTABLE)


