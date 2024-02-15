# Compiler
CC := gcc

# Compiler flags
CFLAGS := -Wall -Wextra
LFLAGS := -Ilib -I../src/network -I../src -Llib -ltest

LIB_DIR := lib
BIN_DIR := bin

# Compiled library
LIB := libtest.a

SRC_DIR := ../src ../src/network
SRC_FILES := $(foreach dir, $(SRC_DIR), $(wildcard $(SRC_DIR)/*.c))
BUILD_DIR := build
OBJ_FILES := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

$(info $(SRC_FILES))
$(info $(OBJ_FILES))


TEST_SRC_DIR := test_src
TEST_SRC_FILES := $(wildcard $(TEST_SRC_DIR)/*.c)
TEST_BUILD_DIR := build_test
TEST_OBJ_FILES := $(patsubst $(TEST_SRC_DIR)/%.c,$(TEST_BUILD_DIR)/%.test.o,$(TEST_SRC_FILES))

TEST_BIN_DIR := bin
TEST_TARGET_FILES := $(patsubst $(TEST_SRC_DIR)/%.c,$(TEST_BIN_DIR)/%.test,$(TEST_SRC_FILES))

all: $(BIN_DIR) $(TEST_TARGET_FILES)

# Rule to build test binaries
$(TEST_BIN_DIR)/%.test: $(TEST_BUILD_DIR)/%.test.o $(LIB) $(OBJ_FILES) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ_FILES) $< -o $@ $(LFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

# Compile source files to object files
$(TEST_BUILD_DIR)/%.test.o: $(TEST_SRC_DIR)/%.c | $(TEST_BUILD_DIR)
	$(CC) -c $< -o $@ $(LFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

# Rule to build the library
$(LIB): $(LIB_DIR)/unity.c $(LIB_DIR)/fff.h $(LIB_DIR)/unity.h $(LIB_DIR)/unity_internals.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(LIB_DIR)/unity.c -o $(BUILD_DIR)/unity.o
	ar rcs $@ $(BUILD_DIR)/unity.o
	mv $@ $(LIB_DIR)/


# Clean
clean:
	@echo "Cleaning up..."
	rm -rf $(BIN_DIR) $(BUILD_DIR) $(TEST_BUILD_DIR) *.o
	@echo "Clean complete."

.PHONY: all clean
