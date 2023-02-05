CC = gcc
CFLAGS = -Wall -g

BIN_DIR = ./bin
SRC_DIR = ./src
OBJ_DIR = ./obj

OUTPUT = $(BIN_DIR)/dproject

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(SRC_DIR)/*.c))

all: clean dproject

dproject: $(OBJS)
		$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
		$(CC) $(CFLAGS) -c $< -o $@

install: dproject
		mkdir -p $(DESTDIR)/usr/local/bin
		cp $(OUTPUT) $(DESTDIR)/usr/local/bin/dproject

uninstall:
		rm -f $(DESTDIR)/usr/local/bin/dproject

clean:
	rm -f $(OBJ_DIR)/*.o $(OUTPUT)
