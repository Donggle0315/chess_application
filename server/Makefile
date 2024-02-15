# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g
DEPFLAGS = -MMD -MP

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
TARGETDIR = bin

# Source files
SRCS := $(shell find $(SRCDIR) -name '*.c')

# Object files
OBJS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRCS:.c=.o))

# Target executable
TARGET = $(TARGETDIR)/server

# Include directories
INC := -I$(INCDIR)

# Get all subdirectories of ../include
INC_SUBDIRS := $(shell find $(INCDIR) -type d)

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INC) -o $@ $^

# Rule to compile source files into object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)/$(dir $<)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(addprefix -I,$(INC_SUBDIRS)) -c -o $@ $<

# Include automatically generated dependency files
-include $(OBJS:.o=.d)

# Clean rule
clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.d $(TARGET)

.PHONY: all clean
