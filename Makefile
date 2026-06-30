# Compiler
CC = gcc

# Warning flags
CFLAGS = -Wall

# Source files
SRC = main.c encode.c decode.c

# Output executable
TARGET = stego

.PHONY: all encode decode clean

# Default target
all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Run encoding example
encode:
	./$(TARGET) -e beautiful.bmp secret.txt stego.bmp

# Run decoding example
decode:
	./$(TARGET) -d stego.bmp output

# Remove executable
clean:
	rm -f $(TARGET)