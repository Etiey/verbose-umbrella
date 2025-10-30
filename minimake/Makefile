CC = gcc
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla
SRC_DIR = src
TEST_DIR = tests

#Source files and objects
MAIN_SRC = $(SRC_DIR)/main.c
MAIN_OBJ = main.o

#Target executable
TARGET = minimake

#Default target
all: $(TARGET)

#Compile object files
main.o: $(MAIN_SRC)
	$(CC) $(CFLAGS) -c $(MAIN_SRC) -o main.o

#Link executable
$(TARGET): $(MAIN_OBJ)
	$(CC) $(CFLAGS) $(MAIN_OBJ) -o $(TARGET)

#Run tests
check: $(TARGET)
	echo "Running tests..."
	cd $(TEST_DIR) && ./run_tests.sh

#Clean build artifacts
clean:
	rm -f main.o $(TARGET)

#Mark phony targets
.PHONY: all check clean

