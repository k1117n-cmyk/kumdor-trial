CC := cc
CFLAGS := -Wall -Wextra -pedantic -Iinclude
TARGET := kumdor_01
SOURCES := kumdor_01.c src/game.c src/stages.c
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
SOURCES += src/bgm_player.m
LDFLAGS += -framework Foundation -framework AVFoundation
endif

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) include/game.h
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o src/*.o *.out
