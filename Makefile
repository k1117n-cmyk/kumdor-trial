CC := cc
CFLAGS := -Wall -Wextra -pedantic
TARGET := kumdor_01
SOURCE := kumdor_01.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o *.out
