# Define the compiler
CC = gcc

# Define the target executable
TARGET = p2

# Define the source files
SRCS = p2.c

# Define the object files
OBJS = $(SRCS:.c=.o)

# Define the compiler flags
CFLAGS = -g -Wall

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
    $(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to build the object files
%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
    rm -f $(TARGET) $(OBJS)

# Run the program
run: $(TARGET)
    ./$(TARGET)

.PHONY: all clean run