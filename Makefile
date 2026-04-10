# 1. Variables
CC      := gcc
CFLAGS  := -Wall -Wextra -Iinclude -g
# -DPRINT_ERROS
TARGET  := CPTM

# 2. Folders
SRC_DIR := src
OBJ_DIR := obj

# 3. Files
# Automatically find all .c files in src/
SRCS    := $(wildcard $(SRC_DIR)/*.c)
# Transform src/file.c into obj/file.o
OBJS    := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# 4. Main Rules
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# Compile .c to .o
# The | $(OBJ_DIR) is an "order-only prerequisite" 
# It ensures the folder exists before compiling
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create the obj directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

run:
	./$(TARGET) 

.PHONY: all clean
