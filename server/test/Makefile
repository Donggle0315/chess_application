CC = gcc
CFLAGS = -Wall -Wextra -g
TESTDIR = ./test_src
BINDIR = bin
INCDIR = ../include

MAINOBJDIR = ../build
OBJS := $(shell find $(MAINOBJDIR) -name '*.o' | grep -v main.o)


# List all the test source files in /test
TEST_SRCS := $(wildcard $(TESTDIR)/*.c)
# Extracts only the filenames without extensions
TEST_BASENAMES := $(notdir $(basename $(TEST_SRCS)))

# List all the corresponding test executables in ../bin
TEST_EXES := $(addprefix $(BINDIR)/,$(addsuffix .test,$(TEST_BASENAMES)))

# Get all subdirectories of ../include
INC_SUBDIRS := $(shell find $(INCDIR) -type d)



# Default target
all: project tests

# Build the project files in ../src
project:
	cd .. && make

# Build the individual test files in /test
tests: $(TEST_EXES)

# Pattern rule to build individual test executables
$(BINDIR)/%.test: $(TESTDIR)/%.c $(OBJS) build/unity.o | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) build/unity.o $(addprefix -I,$(INC_SUBDIRS)) -Iinclude

build/unity.o: include/unity.h include/unity.c
	$(CC) $(CFLAGS) -o unity.o

$(BINDIR):
	mkdir -p $(BINDIR)

# Clean rule
clean:
	rm -r $(BINDIR)
