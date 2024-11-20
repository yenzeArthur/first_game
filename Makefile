CC     = gcc
CFLAGS = -Wall -Werror
TARGET = first_game
FILE   = first_game.c

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(FILE)
	$(CC) $(CFLAGS) $(FILE) -o $(TARGET)

clean:
	rm -f ./$(TARGET)
