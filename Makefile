CC=cc
CFLAGS=-std=c23 -Wall -Wextra -O2
BIN=app

all: $(BIN)

$(BIN): main.c
	$(CC) $(CFLAGS) main.c -o $(BIN) $(shell pkg-config --cflags --libs sdl2)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
