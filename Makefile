CC := cc
CFLAGS := -Wall -Wextra -pedantic -Iinclude
TARGET := kumdor_01
SOURCES := kumdor_01.c src/game.c src/stages.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) include/game.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o src/*.o *.out
