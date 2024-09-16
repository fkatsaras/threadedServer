# Compiler 
CC = gcc

CFLAGS = -pthread

TARGET = server

SRCS = server.c

OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Create target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)


# Create object files
%.0: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup 
clean:
	rm -rf $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
