CC = gcc
CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS = -std=c99 -Wall -Wextra -Werror -fvisibility=hidden -fPIC
LDFLAGS = -shared

OBJ_DIR = obj
TARGET = libmalloc.so

OBJS = $(OBJ_DIR)/malloc.o \
       $(OBJ_DIR)/alignment.o \
       $(OBJ_DIR)/beware_overflow.o \
       $(OBJ_DIR)/my_recycler.o \
       $(OBJ_DIR)/page_begin.o

all: library

library: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/malloc.o: src/malloc.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/alignment.o: src/alignment.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/beware_overflow.o: src/beware_overflow.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/my_recycler.o: src/my_recycler.c | $(OBJ_DIR)
        $(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/page_begin.o: src/page_begin.c | $(OBJ_DIR)
        $(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all library clean
