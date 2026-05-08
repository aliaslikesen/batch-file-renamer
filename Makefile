CC = gcc
CFLAGS = -std=c17 -Wall -Wextra
TARGET = batch-file-renamer
SRC = src/main.c

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) $(TARGET).exe
