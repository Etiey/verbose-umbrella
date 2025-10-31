# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

# Directories
SRC_DIR = src
OBJ_DIR = obj

MINI = minimake

# Source files
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/parser.c \
       $(SRC_DIR)/rules.c \
       $(SRC_DIR)/variables.c \
       $(SRC_DIR)/executor.c \
       $(SRC_DIR)/utils.c

# Object files (derived from source files)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Default target: build the minimake binary
all: $(MINI)

# Link object files to create the binary
$(MINI): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Run the test suite
check:
	echo "Running tests..."
	if [ -d tests ] && [ -f tests/run_tests.sh ]; then \
		cd tests && ./run_tests.sh; \
	else \
		echo "No tests found in tests/ directory"; \
	fi

# Clean up the generated files
clean:
	rm -rf $(OBJ_DIR) $(MINI)

# Phony targets 
.PHONY: all check clean
